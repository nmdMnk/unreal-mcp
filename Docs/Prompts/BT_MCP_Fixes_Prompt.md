# Claude Code 引き継ぎ指示書: BehaviorTree MCP 3つの問題修正

## コンテキスト
- **プロジェクト**: spirrow-unrealwise
- **フェーズ**: Phase 4-4 BehaviorTree修正

## 概要
BehaviorTree MCPツールに3つの問題がある。これらを修正する。

## 問題と原因

### 問題1: ノード接続が反映されない
- `parent_node_id`を指定してノードを作成しても、親子関係が設定されない
- **原因**: `HandleAddBTCompositeNode`と`HandleAddBTTaskNode`で`parent_node_id`パラメータを受け取っていない

### 問題2: BlackboardKeyの設定が無視される
- `set_bt_node_property`で`BlackboardKey`を設定しても反映されない
- **原因**: `FBlackboardKeySelector`構造体の特別処理がない。`SelectedKeyName`だけでなく`SelectedKeyID`も設定が必要

### 問題3: ノードIDが常に同じ値を返す
- 複数のSequence作成 → 全て`BTComposite_Sequence_0`になる
- **原因**: `NewObject`で`NAME_None`を指定しているため、グラフ内で既存の`_0`が検索される

---

## 実装タスク

### 1. 問題1修正: ノード接続の自動化

#### 修正ファイル: `SpirrowBridgeAICommands_BTNodeCreation.cpp`

**HandleAddBTCompositeNode に追加:**

```cpp
// パラメータ取得部分に追加
FString ParentNodeId;
FSpirrowBridgeCommonUtils::GetOptionalString(
    Params, TEXT("parent_node_id"), ParentNodeId, TEXT(""));

int32 ChildIndex = -1;
double ChildIndexDouble = -1.0;
FSpirrowBridgeCommonUtils::GetOptionalNumber(
    Params, TEXT("child_index"), ChildIndexDouble, -1.0);
ChildIndex = static_cast<int32>(ChildIndexDouble);
```

**ノード作成後、保存前に追加（`FinalizeAndSaveBTGraph`の前）:**

```cpp
// ★ 親ノードへの自動接続 ★
if (!ParentNodeId.IsEmpty())
{
    UBehaviorTreeGraphNode* ParentGraphNode = nullptr;
    
    if (ParentNodeId.Equals(TEXT("Root"), ESearchCase::IgnoreCase))
    {
        ParentGraphNode = FindRootGraphNode(BTGraph);
    }
    else
    {
        ParentGraphNode = FindGraphNodeById(BTGraph, ParentNodeId);
    }
    
    if (ParentGraphNode && GraphNode)
    {
        ConnectGraphNodes(ParentGraphNode, GraphNode);
    }
}
```

**HandleAddBTTaskNode にも同様の修正を追加**

---

### 2. 問題3修正: ユニークなノードID生成

#### 修正ファイル: `SpirrowBridgeAICommands_BTNodeCreation.cpp`

**ヘルパー関数を追加（ファイル先頭のstatic関数群の後）:**

```cpp
/**
 * Generate a unique node name based on class name and existing node count
 */
static FName GenerateUniqueNodeName(UBehaviorTreeGraph* BTGraph, UClass* NodeClass)
{
    FString BaseClassName = NodeClass->GetName();
    
    // Count existing nodes of the same class
    int32 Index = 0;
    for (UEdGraphNode* Node : BTGraph->Nodes)
    {
        UBehaviorTreeGraphNode* BTGraphNode = Cast<UBehaviorTreeGraphNode>(Node);
        if (BTGraphNode && BTGraphNode->NodeInstance && 
            BTGraphNode->NodeInstance->GetClass() == NodeClass)
        {
            Index++;
        }
    }
    
    // Generate unique name like "BTComposite_Selector_1"
    return FName(*FString::Printf(TEXT("%s_%d"), *BaseClassName, Index));
}
```

**NewObjectの呼び出しを修正:**

```cpp
// 変更前:
UBTCompositeNode* RuntimeNode = NewObject<UBTCompositeNode>(
    CompositeNode,
    NodeClass,
    NAME_None,           // ← ここを変更
    RF_Transactional
);

// 変更後:
FName UniqueName = GenerateUniqueNodeName(BTGraph, NodeClass);
UBTCompositeNode* RuntimeNode = NewObject<UBTCompositeNode>(
    CompositeNode,
    NodeClass,
    UniqueName,          // ← ユニークな名前を使用
    RF_Transactional
);
```

**全てのNewObject呼び出し箇所を修正:**
- `HandleAddBTCompositeNode`: Composite, SimpleParallel の両方
- `HandleAddBTTaskNode`: Task, SubtreeTask の両方
- `HandleAddBTDecoratorNode`
- `HandleAddBTServiceNode`

---

### 3. 問題2修正: BlackboardKey設定の特別処理

#### 修正ファイル: `SpirrowBridgeAICommands_BTNodeOperations.cpp`

**必要なincludeを追加:**

```cpp
#include "BehaviorTree/BehaviorTreeTypes.h"      // FBlackboardKeySelector
#include "BehaviorTree/BlackboardData.h"
```

**HandleSetBTNodePropertyに特別処理を追加（プロパティ設定の前）:**

```cpp
// ★ BlackboardKey の特別処理 ★
if (PropertyName == TEXT("BlackboardKey") || PropertyName.EndsWith(TEXT("BlackboardKey")))
{
    // FBlackboardKeySelector を直接設定
    FStructProperty* StructProp = CastField<FStructProperty>(
        TargetNode->GetClass()->FindPropertyByName(*PropertyName));
    
    if (StructProp && StructProp->Struct == FBlackboardKeySelector::StaticStruct())
    {
        FBlackboardKeySelector* KeySelector = StructProp->ContainerPtrToValuePtr<FBlackboardKeySelector>(TargetNode);
        
        if (KeySelector)
        {
            // キー名を取得
            FString KeyName;
            if (PropertyValuePtr->Type == EJson::String)
            {
                KeyName = PropertyValuePtr->AsString();
            }
            else if (PropertyValuePtr->Type == EJson::Object)
            {
                TSharedPtr<FJsonObject> KeyObj = PropertyValuePtr->AsObject();
                KeyObj->TryGetStringField(TEXT("SelectedKeyName"), KeyName);
            }
            
            if (!KeyName.IsEmpty())
            {
                // BehaviorTreeからBlackboardを取得
                UBlackboardData* Blackboard = BehaviorTree->BlackboardAsset;
                if (Blackboard)
                {
                    // キーIDを検索
                    FBlackboard::FKey KeyID = Blackboard->GetKeyID(FName(*KeyName));
                    
                    if (KeyID != FBlackboard::InvalidKey)
                    {
                        // キー設定
                        KeySelector->SelectedKeyName = FName(*KeyName);
                        
                        // ★重要: NeedsResolving() = true の場合、InitSelectedKey() を呼ぶ
                        KeySelector->ResolveSelectedKey(*Blackboard);
                        
                        UE_LOG(LogTemp, Display, TEXT("Set BlackboardKey: %s -> KeyID: %d"), 
                            *KeyName, KeyID);
                        
                        // グラフ更新と保存
                        FinalizeAndSaveBTGraphInternal(BTGraph, BehaviorTree);
                        
                        // レスポンス
                        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject());
                        Result->SetBoolField(TEXT("success"), true);
                        Result->SetStringField(TEXT("behavior_tree_name"), BehaviorTreeName);
                        Result->SetStringField(TEXT("node_id"), NodeId);
                        Result->SetStringField(TEXT("property_name"), PropertyName);
                        Result->SetStringField(TEXT("key_name"), KeyName);
                        return Result;
                    }
                    else
                    {
                        // 利用可能なキーをリスト
                        TArray<FString> AvailableKeys;
                        for (const FBlackboardEntry& Key : Blackboard->Keys)
                        {
                            AvailableKeys.Add(Key.EntryName.ToString());
                        }
                        
                        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
                            ESpirrowErrorCode::PropertySetFailed,
                            FString::Printf(TEXT("Blackboard key not found: %s. Available keys: %s"), 
                                *KeyName, *FString::Join(AvailableKeys, TEXT(", "))));
                    }
                }
                else
                {
                    return FSpirrowBridgeCommonUtils::CreateErrorResponse(
                        ESpirrowErrorCode::AssetNotFound,
                        TEXT("BehaviorTree has no Blackboard asset assigned"));
                }
            }
        }
    }
}

// 既存のSetObjectProperty呼び出し...
```

---

## ファイル構成

| ファイル | 修正内容 |
|----------|----------|
| `SpirrowBridgeAICommands_BTNodeCreation.cpp` | 問題1, 3の修正 |
| `SpirrowBridgeAICommands_BTNodeOperations.cpp` | 問題2の修正 |

---

## 完了条件

- [ ] ノード作成時に`parent_node_id`を指定すると自動的に親に接続される
- [ ] 複数の同種ノードを作成しても、それぞれユニークなIDが返される
- [ ] `set_bt_node_property`で`BlackboardKey`を設定すると正しく反映される
- [ ] ビルドが通る
- [ ] 既存のBTツールが正常に動作する

---

## テスト手順

### テスト1: ノード接続

```python
# BT作成
create_behavior_tree(name="BT_Test", blackboard_name="BB_Test")

# Root直下にSelector作成（自動接続）
add_bt_composite_node(
    behavior_tree_name="BT_Test",
    node_type="Selector",
    parent_node_id="Root"
)
# 期待: SelectorがRootに接続される
```

### テスト2: ユニークなノードID

```python
# 複数のSequence作成
result1 = add_bt_composite_node(behavior_tree_name="BT_Test", node_type="Sequence", parent_node_id="...")
result2 = add_bt_composite_node(behavior_tree_name="BT_Test", node_type="Sequence", parent_node_id="...")
# 期待: result1["node_id"] != result2["node_id"]
```

### テスト3: BlackboardKey設定

```python
# MoveToタスク作成
result = add_bt_task_node(behavior_tree_name="BT_Test", task_type="BTTask_MoveTo", parent_node_id="...")

# BlackboardKey設定
set_bt_node_property(
    behavior_tree_name="BT_Test",
    node_id=result["node_id"],
    property_name="BlackboardKey",
    property_value="TargetActor"
)
# 期待: MoveToタスクのBlackboardKeyが"TargetActor"になる
```

---

## 注意事項

1. **FBlackboardKeySelector の設定**
   - `SelectedKeyName` だけでなく `ResolveSelectedKey()` を呼ぶ必要がある
   - これにより `SelectedKeyID` と `SelectedKeyType` が正しく設定される

2. **ユニークな名前生成**
   - グラフ内の同クラスノード数をカウントしてインデックスを付ける
   - `BTComposite_Selector_0`, `BTComposite_Selector_1` のような形式

3. **親ノード接続**
   - `FindGraphNodeById` と `ConnectGraphNodes` の既存関数を再利用
   - Rootへの接続は特別処理（`FindRootGraphNode`を使用）
