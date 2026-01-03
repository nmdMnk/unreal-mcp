"""
SpirrowUnrealWise エラーコード定義

C++側 (ESpirrowErrorCode) と対応するPython側のエラーコード
"""

from enum import IntEnum
from dataclasses import dataclass
from typing import Optional, Dict, Any


class ErrorCode(IntEnum):
    """エラーコード定義 - C++側と同期"""
    
    # General errors (1000-1099)
    SUCCESS = 0
    UNKNOWN_ERROR = 1000
    INVALID_PARAMS = 1001
    MISSING_REQUIRED_PARAM = 1002
    INVALID_PARAM_TYPE = 1003
    INVALID_PARAM_VALUE = 1004
    
    # Asset errors (1100-1199)
    ASSET_NOT_FOUND = 1100
    ASSET_LOAD_FAILED = 1101
    ASSET_ALREADY_EXISTS = 1102
    ASSET_CREATION_FAILED = 1103
    ASSET_DELETE_FAILED = 1104
    INVALID_ASSET_PATH = 1105
    
    # Blueprint errors (1200-1299)
    BLUEPRINT_NOT_FOUND = 1200
    BLUEPRINT_COMPILE_FAILED = 1201
    BLUEPRINT_INVALID_CLASS = 1202
    EVENT_GRAPH_NOT_FOUND = 1203
    NODE_CREATION_FAILED = 1204
    NODE_CONNECTION_FAILED = 1205
    PIN_NOT_FOUND = 1206
    VARIABLE_NOT_FOUND = 1207
    FUNCTION_NOT_FOUND = 1208
    
    # Widget errors (1300-1399)
    WIDGET_NOT_FOUND = 1300
    WIDGET_ELEMENT_NOT_FOUND = 1301
    WIDGET_CREATION_FAILED = 1302
    WIDGET_TREE_NOT_FOUND = 1303
    CANVAS_PANEL_NOT_FOUND = 1304
    ANIMATION_NOT_FOUND = 1305
    
    # Actor errors (1400-1499)
    ACTOR_NOT_FOUND = 1400
    ACTOR_SPAWN_FAILED = 1401
    COMPONENT_NOT_FOUND = 1402
    PROPERTY_NOT_FOUND = 1403
    PROPERTY_SET_FAILED = 1404
    
    # GAS errors (1500-1599)
    GAMEPLAY_TAG_INVALID = 1500
    GAMEPLAY_EFFECT_FAILED = 1501
    GAMEPLAY_ABILITY_FAILED = 1502


# エラーコードのカテゴリ
ERROR_CATEGORIES = {
    range(1000, 1100): "General",
    range(1100, 1200): "Asset",
    range(1200, 1300): "Blueprint",
    range(1300, 1400): "Widget",
    range(1400, 1500): "Actor",
    range(1500, 1600): "GAS",
}


def get_error_category(code: int) -> str:
    """エラーコードからカテゴリを取得"""
    for code_range, category in ERROR_CATEGORIES.items():
        if code in code_range:
            return category
    return "Unknown"


@dataclass
class SpirrowError(Exception):
    """SpirrowUnrealWise エラー"""
    code: ErrorCode
    message: str
    details: Optional[Dict[str, Any]] = None
    
    def __str__(self):
        return f"[{self.code.name} ({self.code.value})] {self.message}"
    
    @property
    def category(self) -> str:
        return get_error_category(self.code.value)
    
    def to_dict(self) -> Dict[str, Any]:
        result = {
            "error_code": self.code.value,
            "error_name": self.code.name,
            "error": self.message,
            "category": self.category,
        }
        if self.details:
            result["details"] = self.details
        return result


def parse_error_response(response: Dict[str, Any]) -> Optional[SpirrowError]:
    """レスポンスからエラーを解析"""
    if response.get("success", True):
        return None
    
    error_code = response.get("error_code", ErrorCode.UNKNOWN_ERROR)
    error_message = response.get("error", "Unknown error")
    details = response.get("details")
    
    try:
        code = ErrorCode(error_code)
    except ValueError:
        code = ErrorCode.UNKNOWN_ERROR
    
    return SpirrowError(code=code, message=error_message, details=details)


# ユーザーフレンドリーなエラーメッセージ
ERROR_MESSAGES = {
    ErrorCode.BLUEPRINT_NOT_FOUND: "指定されたBlueprintが見つかりません。パスと名前を確認してください。",
    ErrorCode.WIDGET_NOT_FOUND: "指定されたWidget Blueprintが見つかりません。",
    ErrorCode.MISSING_REQUIRED_PARAM: "必須パラメータが不足しています。",
    ErrorCode.ASSET_NOT_FOUND: "指定されたアセットが見つかりません。",
    ErrorCode.NODE_CREATION_FAILED: "ノードの作成に失敗しました。",
    ErrorCode.COMPONENT_NOT_FOUND: "指定されたコンポーネントが見つかりません。",
}


def get_friendly_message(code: ErrorCode) -> str:
    """エラーコードからユーザーフレンドリーなメッセージを取得"""
    return ERROR_MESSAGES.get(code, f"エラーが発生しました (コード: {code.value})")
