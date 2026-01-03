#!/usr/bin/env python
"""
Quick Smoke Test - 主要機能の簡易動作確認

pytest不要で実行可能なスタンドアロンテスト
"""

import sys
import os

# パス追加
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from test_framework import UnrealMCPClient, TestSuite, assert_success
import uuid


def generate_name(prefix: str = "Test") -> str:
    """ユニーク名生成"""
    return f"{prefix}_{uuid.uuid4().hex[:8]}"


def run_smoke_tests():
    """スモークテスト実行"""
    print("=" * 60)
    print("SpirrowUnrealWise Smoke Test")
    print("=" * 60)
    
    client = UnrealMCPClient()
    suite = TestSuite(client)
    
    try:
        # Test 1: Widget作成
        print("\n[1/5] Widget作成テスト...")
        widget_name = generate_name("WBP_Smoke")
        result = suite.run_command("create_umg_widget_blueprint", {
            "widget_name": widget_name,
            "path": "/Game/Test"
        })
        if result.success:
            print(f"  ✅ OK ({result.duration_ms:.0f}ms)")
            suite.add_cleanup("delete_asset", {"asset_path": f"/Game/Test/{widget_name}"})
        else:
            print(f"  ❌ FAILED: {result.error}")
        
        # Test 2: Text追加
        print("\n[2/5] Text追加テスト...")
        result = suite.run_command("add_text_to_widget", {
            "widget_name": widget_name,
            "text_name": "SmokeText",
            "text": "Smoke Test",
            "path": "/Game/Test"
        })
        if result.success:
            print(f"  ✅ OK ({result.duration_ms:.0f}ms)")
        else:
            print(f"  ❌ FAILED: {result.error}")
        
        # Test 3: Button追加
        print("\n[3/5] Button追加テスト...")
        result = suite.run_command("add_button_to_widget", {
            "widget_name": widget_name,
            "button_name": "SmokeButton",
            "text": "Click",
            "path": "/Game/Test"
        })
        if result.success:
            print(f"  ✅ OK ({result.duration_ms:.0f}ms)")
        else:
            print(f"  ❌ FAILED: {result.error}")
        
        # Test 4: Blueprint作成
        print("\n[4/5] Blueprint作成テスト...")
        bp_name = generate_name("BP_Smoke")
        result = suite.run_command("create_blueprint", {
            "name": bp_name,
            "parent_class": "Actor",
            "path": "/Game/Test"
        })
        if result.success:
            print(f"  ✅ OK ({result.duration_ms:.0f}ms)")
            suite.add_cleanup("delete_asset", {"asset_path": f"/Game/Test/{bp_name}"})
        else:
            print(f"  ❌ FAILED: {result.error}")
        
        # Test 5: PrintStringノード追加
        print("\n[5/5] PrintStringノード追加テスト...")
        result = suite.run_command("add_print_string_node", {
            "blueprint_name": bp_name,
            "message": "Smoke Test",
            "path": "/Game/Test"
        })
        if result.success:
            print(f"  ✅ OK ({result.duration_ms:.0f}ms)")
        else:
            print(f"  ❌ FAILED: {result.error}")
        
        # サマリー
        summary = suite.get_summary()
        print("\n" + "=" * 60)
        print(f"結果: {summary['passed']}/{summary['total']} テスト成功")
        print(f"成功率: {summary['pass_rate']:.0f}%")
        print(f"平均実行時間: {summary['avg_duration_ms']:.0f}ms")
        
        if summary['failures']:
            print("\n失敗したテスト:")
            for f in summary['failures']:
                print(f"  - {f['command']}: {f['error']}")
        
        print("=" * 60)
        
        return summary['failed'] == 0
        
    finally:
        # クリーンアップ
        print("\nクリーンアップ実行中...")
        suite.cleanup()
        print("完了")


if __name__ == "__main__":
    success = run_smoke_tests()
    sys.exit(0 if success else 1)
