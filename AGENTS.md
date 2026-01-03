# AGENTS.md - spirrow-unrealwise MCP

このドキュメントは、AIエージェントがspirrow-unrealwise MCPツールを使用する際のガイドラインです。

---

## 概要

spirrow-unrealwiseは、Unreal Engine 5とMCP（Model Context Protocol）を接続し、LLMからUEエディタを操作するためのツール群です。

**主な用途**:
- Blueprint作成・編集
- アクター操作
- ノードグラフ構築
- UMG Widget作成・編集
- GAS（Gameplay Ability System）セットアップ
- マテリアル作成

**バージョン**: 0.6.6+ (Phase C完了)

---

## プロジェクト構造

```
spirrow-unrealwise/
├── MCPGameProject/           # サンプルUnrealプロジェクト
│   └── Plugins/SpirrowBridge/  # C++プラグイン
│       └── Source/SpirrowBridge/
│           ├── Public/Commands/   # ヘッダファイル
│           └── Private/Commands/  # 実装ファイル (18ファイル)
├── Python/
│   ├── tools/               # MCPツール定義
│   │   ├── actor_tools.py
│   │   ├── blueprint_tools.py
│   │   ├── umg_tools.py     # UMG Widget操作
│   │   ├── gas_tools.py     # GAS機能
│   │   ├── material_tools.py
│   │   ├── rag_tools.py     # RAGナレッジ
│   │   ├── config_tools.py
│   │   ├── node_tools.py
│   │   ├── input_tools.py
│   │   └── error_codes.py   # エラーコード定義 🆕
│   ├── tests/               # テストスイート 🆕
│   │   ├── test_framework.py
│   │   ├── test_umg_widgets.py
│   │   ├── test_blueprints.py
│   │   └── smoke_test.py
│   └── unreal_mcp_server.py
├── FEATURE_STATUS.md         # 全ツール動作確認状況
├── AGENTS.md                 # このファイル
└── README.md                 # セットアップガイド
```

---

## ノード配置ルール

### 基本設定

```
水平間隔: 300px
垂直間隔: 150px
起点: [0, 0]
```

### レイアウトパターン

#### 1. 直列（Linear）

```
[Event] → [Node A] → [Node B] → [Node C]
[0, 0]    [300, 0]   [600, 0]   [900, 0]
```

```python
# 計算式
x = index * 300
y = 0
```

#### 2. 分岐（Branch）

```
              → [Node B] [300, 0]
[Event] → [Branch]
              → [Node C] [300, 150]
[0, 0]    [300, 0]
```

分岐後のノードは下方向（+Y）に展開:
```python
# 分岐先の計算
x = parent_x + 300
y = branch_index * 150
```

#### 3. 合流（Merge）

```
[Node A] →
              → [Node C]
[Node B] →
```

合流ノードは最も下の入力ノードのY座標 + 75（中央揃え）に配置。

---

## Blueprint作成のベストプラクティス

### 命名規則

| 種類 | プレフィックス | 例 |
|------|---------------|-----|
| Actor Blueprint | `BP_` | `BP_Enemy`, `BP_Projectile` |
| Widget Blueprint | `WBP_` | `WBP_MainMenu`, `WBP_HUD` |
| Component | なし（説明的な名前） | `CubeMesh`, `RootCollision` |
| GameplayEffect | `GE_` | `GE_Damage`, `GE_HealOverTime` |
| GameplayAbility | `GA_` | `GA_Attack`, `GA_Dash` |

### 作成フロー

#### Actor Blueprint の場合
```python
# 1. Blueprint作成
create_blueprint(name="BP_Example", parent_class="Actor", path="/Game/Blueprints")

# 2. コンポーネント追加
add_component_to_blueprint(blueprint_name="BP_Example", component_type="StaticMeshComponent", 
                          component_name="Mesh", path="/Game/Blueprints")

# 3. メッシュ設定
set_static_mesh_properties(blueprint_name="BP_Example", component_name="Mesh",
                          static_mesh="/Engine/BasicShapes/Cube.Cube", path="/Game/Blueprints")

# 4. イベントノード追加
add_blueprint_event_node(blueprint_name="BP_Example", event_name="ReceiveBeginPlay", path="/Game/Blueprints")

# 5. 関数ノード追加
add_print_string_node(blueprint_name="BP_Example", message="Hello!", path="/Game/Blueprints")

# 6. コンパイル
compile_blueprint(blueprint_name="BP_Example", path="/Game/Blueprints")
```

#### Widget Blueprint の場合
```python
# 1. Widget Blueprint作成
create_umg_widget_blueprint(widget_name="WBP_HUD", path="/Game/UI")

# 2. Text追加
add_text_to_widget(widget_name="WBP_HUD", text_name="TitleText", text="Score: 0",
                  font_size=24, anchor="TopCenter", path="/Game/UI")

# 3. Button追加
add_button_to_widget(widget_name="WBP_HUD", button_name="StartBtn", text="Start",
                    size=[200, 50], anchor="Center", path="/Game/UI")

# 4. ProgressBar追加
add_progressbar_to_widget(widget_name="WBP_HUD", progressbar_name="HealthBar",
                         percent=1.0, fill_color=[0, 1, 0, 1], path="/Game/UI")
```

---

## エラーハンドリング 🆕

### エラーコード体系

Phase Cで構造化エラーコードが追加されました:

| 範囲 | カテゴリ | 例 |
|------|----------|-----|
| 1000-1099 | General | InvalidParams, MissingRequiredParam |
| 1100-1199 | Asset | AssetNotFound, AssetLoadFailed |
| 1200-1299 | Blueprint | BlueprintNotFound, NodeCreationFailed |
| 1300-1399 | Widget | WidgetNotFound, WidgetElementNotFound |
| 1400-1499 | Actor | ActorNotFound, ComponentNotFound |
| 1500-1599 | GAS | GameplayTagInvalid |

### エラーレスポンス形式

```json
{
    "success": false,
    "error_code": 1200,
    "error": "Blueprint not found: BP_Test at /Game/Test",
    "details": {
        "blueprint_name": "BP_Test",
        "path": "/Game/Test",
        "full_path": "/Game/Test/BP_Test.BP_Test"
    }
}
```

### Python側でのエラー処理

```python
from tools.error_codes import ErrorCode, parse_error_response

result = some_mcp_tool(...)
if not result.get("success"):
    error = parse_error_response(result)
    if error.code == ErrorCode.BLUEPRINT_NOT_FOUND:
        print(f"Blueprint見つからず: {error.details}")
```

---

## UMG Widget ツール一覧

### コアツール
| ツール | 説明 |
|--------|------|
| `create_umg_widget_blueprint` | Widget Blueprint作成 |
| `add_widget_to_viewport` | Viewportに追加 (PIE実行中) |

### 基本ウィジェット
| ツール | 説明 |
|--------|------|
| `add_text_to_widget` | TextBlock追加 |
| `add_image_to_widget` | Image追加 |
| `add_progressbar_to_widget` | ProgressBar追加 |

### インタラクティブウィジェット
| ツール | 説明 |
|--------|------|
| `add_button_to_widget` | Button追加 |
| `add_slider_to_widget` | Slider追加 |
| `add_checkbox_to_widget` | CheckBox追加 |
| `add_combobox_to_widget` | ComboBox（ドロップダウン）追加 |
| `add_editabletext_to_widget` | EditableText（テキスト入力）追加 |
| `add_spinbox_to_widget` | SpinBox（数値入力）追加 |
| `add_scrollbox_to_widget` | ScrollBox（スクロールコンテナ）追加 |

### レイアウトツール
| ツール | 説明 |
|--------|------|
| `add_vertical_box_to_widget` | VerticalBox追加 |
| `add_horizontal_box_to_widget` | HorizontalBox追加 |
| `set_widget_slot_property` | Canvas Slot設定 |
| `reparent_widget_element` | 親変更 |
| `remove_widget_element` | 要素削除 |

### アニメーションツール
| ツール | 説明 |
|--------|------|
| `create_widget_animation` | アニメーション作成 |
| `add_animation_track` | トラック追加 |
| `add_animation_keyframe` | キーフレーム追加 |
| `get_widget_animations` | アニメーション一覧取得 |

---

## コマンドハンドラ構成 (18ファイル)

```
SpirrowBridge.cpp                      ← メインルーター
├── SpirrowBridgeEditorCommands.cpp    ← アクター操作
├── SpirrowBridgeBlueprintCommands.cpp ← Blueprint操作（ルーター）
│   ├── BlueprintCoreCommands.cpp      ← 作成/コンパイル/スポーン (6関数)
│   ├── BlueprintComponentCommands.cpp ← コンポーネント/物理 (5関数)
│   └── BlueprintPropertyCommands.cpp  ← クラススキャン/配列 (3関数)
├── SpirrowBridgeBlueprintNodeCommands.cpp ← ノード操作（ルーター）
│   ├── BlueprintNodeCoreCommands.cpp      ← 接続/検索/イベント/関数 (7関数)
│   ├── BlueprintNodeVariableCommands.cpp  ← 変数/Get/Set/Self (6関数)
│   └── BlueprintNodeControlFlowCommands.cpp ← Branch/Delay/Math (8関数)
├── SpirrowBridgeUMGWidgetCommands.cpp     ← Widget追加（ルーター）
│   ├── UMGWidgetCoreCommands.cpp          ← 作成/Viewport (3関数)
│   ├── UMGWidgetBasicCommands.cpp         ← Text/Image/ProgressBar (4関数)
│   └── UMGWidgetInteractiveCommands.cpp   ← Button/Slider等 (7関数)
├── SpirrowBridgeUMGLayoutCommands.cpp     ← レイアウト操作
├── SpirrowBridgeUMGAnimationCommands.cpp  ← アニメーション
├── SpirrowBridgeUMGVariableCommands.cpp   ← Widget変数/バインディング
├── SpirrowBridgeProjectCommands.cpp       ← プロジェクト操作
├── SpirrowBridgeGASCommands.cpp           ← GAS
├── SpirrowBridgeConfigCommands.cpp        ← Config操作
├── SpirrowBridgeMaterialCommands.cpp      ← マテリアル
└── SpirrowBridgeCommonUtils.cpp           ← 共通ユーティリティ + エラーコード
```

---

## 新しいコマンドの追加手順

### チェックリスト（必須）

| # | ファイル | 更新内容 |
|---|----------|----------|
| 1 | `Commands/SpirrowBridge*Commands.h` | 関数宣言 |
| 2 | `Commands/SpirrowBridge*Commands.cpp` | 関数実装 |
| 3 | `Commands/SpirrowBridge*Commands.cpp` | HandleCommand内ルーティング |
| 4 | **`SpirrowBridge.cpp`** | **ExecuteCommand内ルーティング** ⚠️ |
| 5 | `Python/tools/*_tools.py` | Python側ツール定義 |

⚠️ **重要**: #4 を忘れると「Unknown command」エラー！

---

## テストの実行 🆕

### スモークテスト（クイック確認）

```powershell
cd Python
python tests/smoke_test.py
```

### pytestによるテスト

```powershell
cd Python
pip install -e ".[test]"

# 全テスト
python tests/run_tests.py

# カテゴリ別
python tests/run_tests.py -m umg
python tests/run_tests.py -m blueprint

# 詳細出力
python tests/run_tests.py -v
```

---

## rationale パラメータ（設計根拠の自動記録）

### 対象ツール

| ツール | カテゴリ |
|--------|----------|
| `create_blueprint` | blueprint |
| `add_component_to_blueprint` | component |
| `set_physics_properties` | physics |
| `spawn_actor` | level_design |
| `add_blueprint_event_node` | blueprint_event |
| `add_blueprint_function_node` | blueprint_logic |
| `add_blueprint_variable` | blueprint_variable |
| `create_gameplay_effect` | gas_effect |
| `create_gameplay_ability` | gas_ability |

### 使用例

```python
create_blueprint(
    name="BP_Enemy",
    parent_class="Character",
    rationale="敵キャラ用。AIControllerで制御し、NavMeshで移動するためCharacterベース"
)
```

---

## 更新履歴

- 2026-01-03: **Phase C完了** - テストフレームワーク作成、エラーハンドリング強化、error_codes.py追加
- 2026-01-03: Phase 0.6.6 UMGWidgetCommands分割完了 (64KB→3分割)
- 2026-01-03: Phase 0.6.5 BlueprintCommands分割完了 (95KB→3分割, 68KB→3分割)
- 2026-01-02: Enhanced Input Blueprint統合機能実装
- 2026-01-01: find_blueprint_nodes修正、Math/Comparisonノード動作確認
- 2025-12-25: 制御フロー・ユーティリティノードツール追加
- 2025-12-15: GAS機能実装、Config操作対応
- 2025-12-03: 初版作成
