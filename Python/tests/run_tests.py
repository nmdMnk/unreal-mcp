#!/usr/bin/env python
"""
SpirrowUnrealWise Test Runner

MCPサーバーのテストを実行するCLIツール

使用方法:
    # 全テスト実行
    python run_tests.py

    # 特定カテゴリのみ
    python run_tests.py -m umg
    python run_tests.py -m blueprint
    python run_tests.py -m node

    # 統合テストのみ
    python run_tests.py -m integration

    # 詳細出力
    python run_tests.py -v

    # 特定テストファイル
    python run_tests.py test_umg_widgets.py
"""

import sys
import argparse
import subprocess
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description="SpirrowUnrealWise テストランナー")
    parser.add_argument("-m", "--marker", help="実行するテストマーカー (umg, blueprint, node, integration)")
    parser.add_argument("-v", "--verbose", action="store_true", help="詳細出力")
    parser.add_argument("-k", "--keyword", help="キーワードでテストをフィルタ")
    parser.add_argument("--host", default="127.0.0.1", help="MCPサーバーホスト")
    parser.add_argument("--port", type=int, default=55557, help="MCPサーバーポート")
    parser.add_argument("--html", help="HTMLレポート出力パス")
    parser.add_argument("tests", nargs="*", help="実行するテストファイル/ディレクトリ")
    
    args = parser.parse_args()
    
    # テストディレクトリ
    test_dir = Path(__file__).parent
    
    # pytest コマンド構築
    cmd = ["python", "-m", "pytest"]
    
    # テストパス
    if args.tests:
        for t in args.tests:
            cmd.append(str(test_dir / t))
    else:
        cmd.append(str(test_dir))
    
    # オプション
    if args.verbose:
        cmd.append("-v")
    else:
        cmd.append("-q")
    
    if args.marker:
        cmd.extend(["-m", args.marker])
    
    if args.keyword:
        cmd.extend(["-k", args.keyword])
    
    if args.html:
        cmd.extend(["--html", args.html, "--self-contained-html"])
    
    # 標準出力を表示
    cmd.append("-s")
    
    # 実行
    print(f"実行コマンド: {' '.join(cmd)}")
    print("-" * 60)
    
    result = subprocess.run(cmd, cwd=test_dir.parent)
    
    return result.returncode


if __name__ == "__main__":
    sys.exit(main())
