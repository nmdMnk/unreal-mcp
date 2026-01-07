# BugFix: BehaviorTree ノード接続が失敗する問題

## 問題概要

`connect_bt_nodes` で BT ノードを Root に接続しようとすると `Child node not found` エラーが発生する。

## 再現手順

```python
# 1. BT 作成（成功）
create_behavior_tree(name="BT_AIFighter", path="/Game/TrapxTrap/AI")

# 2. Composite ノード追加（成功、node_id が返る）
add_bt_composite_node(
    behavior_tree_name="BT_AIFighter",
    node_type="Selector",
    node_name="RootSelector",
    path="/Game/TrapxTrap/AI"
)
# 返り値: {"node_id": "BTComposite_Selector_0", ...}

# 3. Root に接続（失敗）
connect_bt_nodes(
    behavior_tree_name="BT_AIFighter",
    parent_node_id="Root",
    child_node_id="BTComposite_Selector_0",
    path="/Game/TrapxTrap/AI"
)
# エラー: "Child node not found: BTComposite_Selector_0"
```

## エラー詳細

```
{"status": "error", "error": "Child node not found: BTComposite_Selector_0"}
```

## 確認事項

`get_behavior_tree_structure` の結果：
```json
{
  "success": true,
  "name": "BT_AIFighter",
  "blackboard_name": "BB_AIFighter",
  "blackboard_key_count": 8,
  "has_root_node": false  // ← Root に何も接続されていない
}
```

## 想定原因

### 原因1: ノードがアセットに保存されていない

`add_bt_composite_node` でノードを作成後、メモリ上にのみ存在し、アセットに永続化されていない可能性。

**確認ポイント:**
- `HandleAddBTCompositeNode` 内で `MarkPackageDirty()` が呼ばれているか
- `UPackage::SavePackage()` または `UEditorAssetLibrary::SaveAsset()` が必要か

### 原因2: ノード検索ロジックの問題

`connect_bt_nodes` の `HandleConnectBTNodes` で、まだ Root に接続されていない「浮いている」ノードを検索できていない。

**確認ポイント:**
- ノード検索が `BT->RootNode` からの再帰検索のみになっていないか
- 作成したノードを一時的に保持するマップ（TMap）が必要か

### 原因3: ノード ID の形式不一致

返された `BTComposite_Selector_0` が、実際のノードオブジェクト参照と対応していない。

**確認ポイント:**
- `add_bt_composite_node` で生成する ID と `connect_bt_nodes` で検索する ID が一致するか
- `GetName()` vs `GetFName().ToString()` などの違いはないか

---

## 修正方針

### 方針A: ノードキャッシュの導入

BT に追加されたがまだ接続されていないノードを一時的に保持する仕組み。

```cpp
// SpirrowBridgeBehaviorTreeCommands.h
static TMap<FString, TMap<FString, UBTNode*>> PendingNodes; // BT名 -> (NodeID -> Node)

// add_bt_composite_node で
PendingNodes.FindOrAdd(BTName).Add(NodeId, NewNode);

// connect_bt_nodes で
if (auto* NodeMap = PendingNodes.Find(BTName))
{
    if (UBTNode** Found = NodeMap->Find(ChildNodeId))
    {
        ChildNode = *Found;
    }
}
```

### 方針B: 作成と同時に Root 接続

`add_bt_composite_node` で `is_root=true` オプションを追加し、Root ノードの場合は作成と同時に `BT->RootNode` に設定。

```cpp
if (bIsRoot)
{
    BT->RootNode = NewCompositeNode;
}
```

### 方針C: ノード検索の改善

BT 内の全ノードを走査する関数を実装（Root 未接続ノード含む）。

---

## 調査対象ファイル

1. **SpirrowBridgeBehaviorTreeCommands.cpp**
   - `HandleAddBTCompositeNode` - ノード作成ロジック
   - `HandleConnectBTNodes` - ノード接続ロジック
   - `HandleAddBTTaskNode` - 同様の問題がある可能性

2. **SpirrowBridgeBehaviorTreeCommands.h**
   - ノードキャッシュ用の static 変数が必要か

---

## テスト手順

修正後、以下を確認：

1. **Composite ノード作成 → Root 接続**
```python
add_bt_composite_node(bt_name="BT_Test", node_type="Selector")
connect_bt_nodes(bt_name="BT_Test", parent_node_id="Root", child_node_id="...")
get_behavior_tree_structure(name="BT_Test")  # has_root_node: true
```

2. **子ノードの追加**
```python
add_bt_composite_node(bt_name="BT_Test", node_type="Sequence")
connect_bt_nodes(bt_name="BT_Test", parent_node_id="RootSelector", child_node_id="...")
```

3. **Task ノードの追加**
```python
add_bt_task_node(bt_name="BT_Test", task_type="BTTask_MoveTo")
connect_bt_nodes(bt_name="BT_Test", parent_node_id="ChaseSequence", child_node_id="...")
```

---

## 期待する最終動作

```python
# Root Selector 作成・接続
result = add_bt_composite_node("BT_AIFighter", "Selector", "RootSelector")
connect_bt_nodes("BT_AIFighter", "Root", result["node_id"])

# 子 Sequence 作成・接続
result = add_bt_composite_node("BT_AIFighter", "Sequence", "ChaseSequence")
connect_bt_nodes("BT_AIFighter", "RootSelector", result["node_id"])

# Task 追加
result = add_bt_task_node("BT_AIFighter", "BTTask_MoveTo")
connect_bt_nodes("BT_AIFighter", "ChaseSequence", result["node_id"])

# 構造確認
get_behavior_tree_structure("BT_AIFighter")
# → has_root_node: true, ノード階層が見える
```
