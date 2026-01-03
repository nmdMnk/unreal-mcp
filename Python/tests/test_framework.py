"""
SpirrowUnrealWise Test Framework

MCPサーバーとの通信をラップし、pytestベースのテストを容易にするフレームワーク
"""

import sys
import os
import socket
import json
import logging
from typing import Dict, Any, Optional, List
from dataclasses import dataclass
from contextlib import contextmanager

# tools ディレクトリをパスに追加
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'tools'))

try:
    from error_codes import ErrorCode, SpirrowError, parse_error_response
except ImportError:
    # error_codes がない場合のフォールバック
    ErrorCode = None
    SpirrowError = None
    parse_error_response = None

logger = logging.getLogger("SpirrowTest")


@dataclass
class TestResult:
    """テスト結果を格納するデータクラス"""
    success: bool
    command: str
    params: Dict[str, Any]
    response: Optional[Dict[str, Any]]
    error: Optional[str] = None
    error_code: Optional[int] = None
    duration_ms: float = 0.0
    
    @property
    def error_details(self) -> Optional[Dict[str, Any]]:
        """エラー詳細を取得"""
        if self.response:
            return self.response.get("details")
        return None


class UnrealMCPClient:
    """Unreal Engine MCPサーバーとの通信クライアント"""
    
    def __init__(self, host: str = "127.0.0.1", port: int = 55557, timeout: float = 10.0):
        self.host = host
        self.port = port
        self.timeout = timeout
        self._socket: Optional[socket.socket] = None
    
    def connect(self) -> bool:
        """サーバーに接続"""
        try:
            self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self._socket.settimeout(self.timeout)
            self._socket.connect((self.host, self.port))
            logger.info(f"Connected to {self.host}:{self.port}")
            return True
        except Exception as e:
            logger.error(f"Connection failed: {e}")
            return False
    
    def disconnect(self):
        """サーバーから切断"""
        if self._socket:
            try:
                self._socket.close()
            except:
                pass
            self._socket = None
    
    def send_command(self, command: str, params: Dict[str, Any]) -> TestResult:
        """コマンドを送信してレスポンスを取得"""
        import time
        start_time = time.time()
        
        try:
            # 新しい接続を作成（各コマンドごと）
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(self.timeout)
            sock.connect((self.host, self.port))
            
            try:
                # コマンド送信
                command_obj = {"type": command, "params": params}
                command_json = json.dumps(command_obj)
                logger.debug(f"Sending: {command_json}")
                sock.sendall(command_json.encode('utf-8'))
                
                # レスポンス受信
                chunks = []
                while True:
                    chunk = sock.recv(4096)
                    if not chunk:
                        break
                    chunks.append(chunk)
                    
                    # JSONパース試行
                    try:
                        data = b''.join(chunks)
                        json.loads(data.decode('utf-8'))
                        break  # 完全なJSON受信完了
                    except json.JSONDecodeError:
                        continue
                
                # レスポンスパース
                data = b''.join(chunks)
                response = json.loads(data.decode('utf-8'))
                duration_ms = (time.time() - start_time) * 1000
                
                success = response.get("status") == "success"
                error = response.get("error") if not success else None
                error_code = response.get("error_code") if not success else None
                
                return TestResult(
                    success=success,
                    command=command,
                    params=params,
                    response=response,
                    error=error,
                    error_code=error_code,
                    duration_ms=duration_ms
                )
                
            finally:
                sock.close()
                
        except Exception as e:
            duration_ms = (time.time() - start_time) * 1000
            return TestResult(
                success=False,
                command=command,
                params=params,
                response=None,
                error=str(e),
                error_code=None,
                duration_ms=duration_ms
            )
    
    @contextmanager
    def session(self):
        """コンテキストマネージャーでセッション管理"""
        try:
            yield self
        finally:
            self.disconnect()


class TestSuite:
    """テストスイートの基底クラス"""
    
    def __init__(self, client: UnrealMCPClient):
        self.client = client
        self.results: List[TestResult] = []
        self.cleanup_actions: List[Dict[str, Any]] = []
    
    def run_command(self, command: str, params: Dict[str, Any], 
                    expected_success: bool = True) -> TestResult:
        """コマンドを実行し結果を記録"""
        result = self.client.send_command(command, params)
        self.results.append(result)
        
        if expected_success and not result.success:
            logger.warning(f"Expected success but got failure: {command}")
            logger.warning(f"Error [{result.error_code}]: {result.error}")
            if result.error_details:
                logger.warning(f"Details: {result.error_details}")
        elif not expected_success and result.success:
            logger.warning(f"Expected failure but got success: {command}")
        
        return result
    
    def add_cleanup(self, command: str, params: Dict[str, Any]):
        """クリーンアップアクションを登録"""
        self.cleanup_actions.append({"command": command, "params": params})
    
    def cleanup(self):
        """登録されたクリーンアップアクションを実行"""
        for action in reversed(self.cleanup_actions):
            try:
                self.client.send_command(action["command"], action["params"])
            except Exception as e:
                logger.warning(f"Cleanup failed: {e}")
        self.cleanup_actions.clear()
    
    def get_summary(self) -> Dict[str, Any]:
        """テスト結果のサマリーを取得"""
        total = len(self.results)
        passed = sum(1 for r in self.results if r.success)
        failed = total - passed
        avg_duration = sum(r.duration_ms for r in self.results) / total if total > 0 else 0
        
        return {
            "total": total,
            "passed": passed,
            "failed": failed,
            "pass_rate": (passed / total * 100) if total > 0 else 0,
            "avg_duration_ms": avg_duration,
            "failures": [
                {
                    "command": r.command, 
                    "error": r.error,
                    "error_code": r.error_code,
                    "details": r.error_details
                }
                for r in self.results if not r.success
            ]
        }


# ユーティリティ関数
def create_test_client(host: str = "127.0.0.1", port: int = 55557) -> UnrealMCPClient:
    """テスト用クライアントを作成"""
    return UnrealMCPClient(host=host, port=port)


def assert_success(result: TestResult, message: str = ""):
    """成功をアサート"""
    error_info = f" [{result.error_code}]" if result.error_code else ""
    full_error = f"{error_info} {result.error}" if result.error else "Unknown error"
    assert result.success, f"{message}: {full_error}" if message else full_error


def assert_failure(result: TestResult, message: str = ""):
    """失敗をアサート"""
    assert not result.success, f"{message}: Expected failure but got success"


def assert_error_code(result: TestResult, expected_code: int, message: str = ""):
    """特定のエラーコードをアサート"""
    assert not result.success, "Expected failure but got success"
    assert result.error_code == expected_code, \
        f"{message}: Expected error code {expected_code}, got {result.error_code}"


def assert_response_has(result: TestResult, key: str, value: Any = None):
    """レスポンスに特定のキーが含まれることをアサート"""
    assert result.response is not None, "Response is None"
    response_result = result.response.get("result", {})
    assert key in response_result, f"Key '{key}' not found in response"
    if value is not None:
        assert response_result[key] == value, f"Expected {key}={value}, got {response_result[key]}"
