# SpirrowUnrealWise UMG Phase 3 継続プロンプト

## プロジェクト情報

- **MCP プロジェクト**: `C:\Users\takahito ito\Documents\Unreal Projects\spirrow-unrealwise`
- **ゲームプロジェクト**: `C:\Users\takahito ito\Documents\Unreal Projects\TrapxTrapCpp 5.7`
- **プラグイン**: `Plugins/SpirrowBridge`
- **テスト用Widget**: `WBP_TT_TrapSelector` (`/Game/TrapxTrap/UI`)

---

## 現在の状況

### 完了済み

**Phase 1 (Designer操作)**: 11ツール ✅
**Phase 2 (変数・関数)**: 5ツール ✅
**Phase 3 (Animation)**: 4ツール ✅
- `create_widget_animation` - アニメーション作成
- `add_animation_track` - Opacity/ColorAndOpacity トラック追加
- `add_animation_keyframe` - キーフレーム追加（Linear/Cubic/Constant）
- `get_widget_animations` - アニメーション一覧取得 ← NEW

**合計: 20ツール実装完了**

### Phase 3 残り機能（未実装）

| 優先度 | ツール | 説明 | プロンプト |
|--------|--------|------|-----------|
| 1️⃣ | `add_widget_array_variable` | 配列型変数追加 | ✅ 作成済み |
| 2️⃣ | RenderTransform トラック | Translation/Scale/Angle 対応 | 未作成 |
| 3️⃣ | `set_widget_array_default` | 配列デフォルト値設定 | 未作成 |

---

## 設計ドキュメント

| ドキュメント | 内容 |
|-------------|------|
| `Docs/UMGPhase3_Animation_Prompt.md` | Phase 3 全体設計 |
| `Docs/UMGPhase3_AnimationTrack_Prompt.md` | Animation Track 詳細 |
| `Docs/UMGPhase3_GetWidgetAnimations_Prompt.md` | アニメーション一覧取得 |
| `Docs/UMGPhase3_ArrayVariable_Prompt.md` | 配列型変数追加 ← 次の実装 |
| `Docs/UMGPhase3_Handover_Prompt.md` | 引き継ぎドキュメント |

---

## 開始手順

1. プロジェクトコンテキスト確認:
```
get_project_context("SpirrowUnrealWise")
```

2. 次の実装プロンプト確認:
```
Docs/UMGPhase3_ArrayVariable_Prompt.md を参照
```

---

## 次に実装: add_widget_array_variable

### 概要

Widget Blueprint に配列型変数（TArray<T>）を追加するツール。

### パラメータ

| パラメータ | 型 | 必須 | 説明 |
|-----------|-----|------|------|
| widget_name | str | ✅ | Widget Blueprint名 |
| variable_name | str | ✅ | 変数名 |
| element_type | str | ✅ | 要素型（String, Integer, Float等） |
| is_exposed | bool | | エディタ公開フラグ |
| category | str | | 変数カテゴリ |
| path | str | | Content Browser パス |

### 実装ポイント

- 既存の `SetupPinType` 関数を再利用
- `ContainerType = EPinContainerType::Array` を設定
- Blueprint コンパイル必須

### 更新ファイル

1. `SpirrowBridgeUMGCommands.h` - 関数宣言
2. `SpirrowBridgeUMGCommands.cpp` - 実装 + ルーティング
3. `SpirrowBridge.cpp` - ExecuteCommand ルーティング
4. `Python/tools/umg_tools.py` - ツール定義

---

## 技術メモ

- Build.cs に `"MovieScene"`, `"MovieSceneTracks"` 追加済み
- UE 5.7 では `TArray64<uint8>` が必要な箇所あり
- 非推奨 API は `PRAGMA_DISABLE_DEPRECATION_WARNINGS` で抑制済み

---

**最終更新**: 2025-12-24
