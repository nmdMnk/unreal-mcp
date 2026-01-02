# spirrow-unrealwise 機能ステータス

## 概要

このドキュメントは、MCPツールの動作確認状況と今後追加予定の機能をまとめたものです。

---

## 確認済み機能

### Actor操作

| ツール | 状態 | 備考 |
|--------|------|------|
| `get_actors_in_level` | ✅ 動作OK | レベル内の全アクター取得 |
| `find_actors_by_name` | 🔲 未確認 | |
| `spawn_actor` | ✅ 動作OK | アクター作成のみ、メッシュ設定は別途必要 |
| `delete_actor` | 🔲 未確認 | |
| `set_actor_transform` | 🔲 未確認 | |
| `get_actor_properties` | ✅ 動作OK | |
| `set_actor_property` | ✅ 動作OK | アクター自体のプロパティを設定。rationale対応 |
| `set_actor_component_property` | ✅ 動作OK | アクターのコンポーネントのプロパティを設定。rationale対応 |
| `rename_actor` | ✅ 動作OK | アクター名変更（ActorLabel/Name両対応） |
| `get_actor_components` | 🔲 未確認 | アクターのコンポーネント一覧取得 |

### Blueprint操作

| ツール | 状態 | 備考 |
|--------|------|------|
| `create_blueprint` | ✅ 動作OK | pathパラメータ対応（デフォルト: /Game/Blueprints） |
| `spawn_blueprint_actor` | ✅ 動作OK | pathパラメータ対応（デフォルト: /Game/Blueprints） |
| `add_component_to_blueprint` | ✅ 動作OK | pathパラメータ対応（デフォルト: /Game/Blueprints） |
| `set_static_mesh_properties` | ✅ 動作OK | Engine標準メッシュで確認。pathパラメータ対応 |
| `set_component_property` | 🔲 未確認 | pathパラメータ対応 |
| `set_physics_properties` | 🔲 未確認 | pathパラメータ対応 |
| `compile_blueprint` | ✅ 動作OK | pathパラメータ対応（デフォルト: /Game/Blueprints） |
| `set_blueprint_property` | 🔲 未確認 | pathパラメータ対応 |

### BPノードグラフ操作

| ツール | 状態 | 備考 |
|--------|------|------|
| `add_blueprint_event_node` | ✅ 動作OK | ReceiveBeginPlay確認。pathパラメータ対応 |
| `add_blueprint_input_action_node` | 🔲 未確認 | pathパラメータ対応 |
| `add_blueprint_function_node` | ✅ 動作OK | target指定が重要（self, KismetSystemLibrary等）。pathパラメータ対応 |
| `connect_blueprint_nodes` | ✅ 動作OK | ピン名: then → execute。pathパラメータ対応 |
| `add_blueprint_variable` | 🔲 未確認 | pathパラメータ対応 |
| `add_blueprint_get_self_component_reference` | 🔲 未確認 | pathパラメータ対応 |
| `add_blueprint_self_reference` | 🔲 未確認 | pathパラメータ対応 |
| `find_blueprint_nodes` | 🔲 未確認 | pathパラメータ対応 |

### UMG Widget操作

#### Phase 1-4: 全29ツール実装完了 ✅

| カテゴリ | ツール数 | 状態 |
|---------|---------|------|
| Core | 3 | ✅ 動作OK |
| Basic | 4 | ✅ 動作OK |
| Interactive | 7 | ✅ 動作OK |
| Layout | 7 | ✅ 実装完了 |
| Variable/Function | 5 | ✅ 実装完了 |
| Animation | 4 | ✅ 実装完了 |

### RAG連携

| ツール | 状態 | 備考 |
|--------|------|------|
| `search_knowledge` | ✅ 動作OK | RAGサーバー連携、意味検索対応 |
| `add_knowledge` | ✅ 動作OK | ナレッジ追加、カテゴリ・タグ対応 |
| `list_knowledge` | ✅ 動作OK | 登録済みナレッジ一覧取得 |
| `delete_knowledge` | ✅ 動作OK | ID指定でナレッジ削除 |

---

## 最新の更新履歴

### 2026-01-03: UMGWidgetCommands 分割リファクタリング完了 (Phase 0.6.6) 🆕

**完了内容**:
- `SpirrowBridgeUMGWidgetCommands.cpp` (64 KB) を3ファイルに分割
- ルーターパターン採用: UMGWidgetCommandsが3つのハンドラへ委譲

**新ファイル構成 - UMGWidget系**:
| ファイル | サイズ | 担当 |
|----------|--------|------|
| `SpirrowBridgeUMGWidgetCoreCommands.cpp` | 7 KB | CreateUMGWidgetBlueprint, AddWidgetToViewport, ParseAnchorPreset（3関数） |
| `SpirrowBridgeUMGWidgetBasicCommands.cpp` | 17 KB | AddTextToWidget, AddTextBlockToWidget, AddImageToWidget, AddProgressBarToWidget（4関数） |
| `SpirrowBridgeUMGWidgetInteractiveCommands.cpp` | 30 KB | AddButtonToWidget, AddSliderToWidget, AddCheckBoxToWidget, AddComboBoxToWidget, AddEditableTextToWidget, AddSpinBoxToWidget, AddScrollBoxToWidget（7関数） |
| `SpirrowBridgeUMGWidgetCommands.cpp` | 1.5 KB | ルーター |

**Python側修正**:
- `umg_tools.py`: `add_button_to_widget_v2` → `add_button_to_widget` コマンド名修正

**テスト結果**:
- 全11コマンド動作確認完了 ✅

**削減効果**:
- 最大ファイルサイズ: 64KB → 30KB (53%削減)
- Phase 0.6累計: Blueprint系6ファイル + UMG系3ファイル分割完了
- 全体最大ファイルサイズ: 166KB → 30KB (82%削減)

---

### 2026-01-03: BlueprintCommands 分割リファクタリング完了 (Phase 0.6.5)

**完了内容**:
- `SpirrowBridgeBlueprintCommands.cpp` (95 KB) を3ファイルに分割
- `SpirrowBridgeBlueprintNodeCommands.cpp` (68 KB) を3ファイルに分割
- オプションB採用: 各ルーターファイルから分割クラスへ委譲

**新ファイル構成 - Blueprint系**:
| ファイル | サイズ | 担当 |
|----------|--------|------|
| `SpirrowBridgeBlueprintCoreCommands.cpp` | 23 KB | 作成/コンパイル/スポーン/複製/グラフ取得（6関数） |
| `SpirrowBridgeBlueprintComponentCommands.cpp` | 26 KB | コンポーネント追加/プロパティ設定/物理（5関数） |
| `SpirrowBridgeBlueprintPropertyCommands.cpp` | 21 KB | クラススキャン/配列プロパティ（3関数） |
| `SpirrowBridgeBlueprintCommands.cpp` | 1.7 KB | ルーター |

**新ファイル構成 - BlueprintNode系**:
| ファイル | サイズ | 担当 |
|----------|--------|------|
| `SpirrowBridgeBlueprintNodeCoreCommands.cpp` | 24 KB | 接続/検索/イベント/関数呼び出し（7関数） |
| `SpirrowBridgeBlueprintNodeVariableCommands.cpp` | 14 KB | 変数/Get/Set/Self参照/InputAction（6関数） |
| `SpirrowBridgeBlueprintNodeControlFlowCommands.cpp` | 21 KB | Branch/Sequence/Delay/Loop/Math/Print（8関数） |
| `SpirrowBridgeBlueprintNodeCommands.cpp` | 1.7 KB | ルーター |

**削減効果**:
- 最大ファイルサイズ: 95KB → 26KB (73%削減)
- 合計6ファイル追加、既存2ファイルはルーターに変換

---

## テスト環境

- **Unreal Engine**: 5.7
- **プロジェクト**: TrapxTrapCpp
- **RAGサーバー**: AIサーバー :8100
- **最終確認日**: 2026-01-03

---

## 凡例

| 記号 | 意味 |
|------|------|
| ✅ | 動作確認済み |
| 🔲 | 未確認 |
| 🆕 | 新規追加 |
