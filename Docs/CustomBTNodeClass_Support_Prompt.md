# Claude Code 引き継ぎ指示書: カスタムC++ BTノードクラス対応

## コンテキスト

- **プロジェクト**: spirrow-unrealwise
- **フェーズ**: Phase G 拡張
- **関連ファイル**: `SpirrowBridgeAICommands_BTNodeHelpers.cpp`

---

## 概要

現在、`add_bt_task_node`、`add_bt_decorator_node`、`add_bt_service_node` は以下のみ対応：
- UE標準のBTノードクラス（ハードコード済み）
- `/Game/AI/Tasks/` 等にあるBlueprintクラス

**問題**: プロジェクト固有のC++カスタムクラス（例: `BTTask_ChaseTarget`）を追加できない。

**解決策**: C++クラスを動的に検索する処理を追加する。

---

## 前提条件

既存の実装:
- `GetBTTaskNodeClass()` - BTTaskクラス取得
- `GetBTDecoratorClass()` - BTDecoratorクラス取得
- `GetBTServiceClass()` - BTServiceクラス取得

これらの関数にC++クラス検索を追加する。

---

## 実装タスク

### 1. `GetBTTaskNodeClass()` の修正

**ファイル**: `MCPGameProject/Plugins/SpirrowBridge/Source/SpirrowBridge/Private/Commands/SpirrowBridgeAICommands_BTNodeHelpers.cpp`

**変更内容**:

```cpp
UClass* FSpirrowBridgeAICommands::GetBTTaskNodeClass(const FString& TypeString)
{
    // 1. 標準タスク（既存のまま）
    if (TypeString == TEXT("BTTask_MoveTo")) return UBTTask_MoveTo::StaticClass();
    if (TypeString == TEXT("BTTask_MoveDirectlyToward")) return UBTTask_MoveDirectlyToward::StaticClass();
    if (TypeString == TEXT("BTTask_Wait")) return UBTTask_Wait::StaticClass();
    if (TypeString == TEXT("BTTask_WaitBlackboardTime")) return UBTTask_WaitBlackboardTime::StaticClass();
    if (TypeString == TEXT("BTTask_PlaySound")) return UBTTask_PlaySound::StaticClass();
    if (TypeString == TEXT("BTTask_PlayAnimation")) return UBTTask_PlayAnimation::StaticClass();
    if (TypeString == TEXT("BTTask_RotateToFaceBBEntry")) return UBTTask_RotateToFaceBBEntry::StaticClass();
    if (TypeString == TEXT("BTTask_RunBehavior")) return UBTTask_RunBehavior::StaticClass();
    if (TypeString == TEXT("BTTask_RunBehaviorDynamic")) return UBTTask_RunBehaviorDynamic::StaticClass();

    // 2. ★追加★ フルパス指定の場合（例: "/Script/TrapxTrapCpp.BTTask_ChaseTarget"）
    if (TypeString.StartsWith(TEXT("/Script/")))
    {
        UClass* FoundClass = LoadClass<UBTTaskNode>(nullptr, *TypeString);
        if (FoundClass)
        {
            return FoundClass;
        }
    }

    // 3. ★追加★ クラス名のみ指定の場合、全モジュールから検索
    {
        // "U" プレフィックスを付けて検索（UBTTask_ChaseTarget）
        FString ClassNameWithU = TypeString.StartsWith(TEXT("U")) ? TypeString : FString::Printf(TEXT("U%s"), *TypeString);
        
        for (TObjectIterator<UClass> It; It; ++It)
        {
            UClass* Class = *It;
            if (Class->GetName() == ClassNameWithU || Class->GetName() == TypeString)
            {
                if (Class->IsChildOf(UBTTaskNode::StaticClass()))
                {
                    return Class;
                }
            }
        }
    }

    // 4. Blueprint検索（既存のまま）
    FString BlueprintPath = FString::Printf(TEXT("/Game/AI/Tasks/%s.%s"), *TypeString, *TypeString);
    UBlueprint* Blueprint = Cast<UBlueprint>(UEditorAssetLibrary::LoadAsset(BlueprintPath));
    if (Blueprint && Blueprint->GeneratedClass)
    {
        return Blueprint->GeneratedClass;
    }

    return nullptr;
}
```

### 2. `GetBTDecoratorClass()` の修正

**同様の変更を追加**:

```cpp
UClass* FSpirrowBridgeAICommands::GetBTDecoratorClass(const FString& TypeString)
{
    // 1. 標準デコレータ（既存のまま）
    if (TypeString == TEXT("BTDecorator_Blackboard")) return UBTDecorator_Blackboard::StaticClass();
    // ... 他の標準クラス ...

    // 2. ★追加★ フルパス指定の場合
    if (TypeString.StartsWith(TEXT("/Script/")))
    {
        UClass* FoundClass = LoadClass<UBTDecorator>(nullptr, *TypeString);
        if (FoundClass)
        {
            return FoundClass;
        }
    }

    // 3. ★追加★ クラス名のみ指定の場合
    {
        FString ClassNameWithU = TypeString.StartsWith(TEXT("U")) ? TypeString : FString::Printf(TEXT("U%s"), *TypeString);
        
        for (TObjectIterator<UClass> It; It; ++It)
        {
            UClass* Class = *It;
            if (Class->GetName() == ClassNameWithU || Class->GetName() == TypeString)
            {
                if (Class->IsChildOf(UBTDecorator::StaticClass()))
                {
                    return Class;
                }
            }
        }
    }

    // 4. Blueprint検索（既存のまま）
    FString BlueprintPath = FString::Printf(TEXT("/Game/AI/Decorators/%s.%s"), *TypeString, *TypeString);
    // ...
}
```

### 3. `GetBTServiceClass()` の修正

**同様の変更を追加**:

```cpp
UClass* FSpirrowBridgeAICommands::GetBTServiceClass(const FString& TypeString)
{
    // 1. 標準サービス（既存のまま）
    if (TypeString == TEXT("BTService_BlackboardBase")) return UBTService_BlackboardBase::StaticClass();
    // ... 他の標準クラス ...

    // 2. ★追加★ フルパス指定の場合
    if (TypeString.StartsWith(TEXT("/Script/")))
    {
        UClass* FoundClass = LoadClass<UBTService>(nullptr, *TypeString);
        if (FoundClass)
        {
            return FoundClass;
        }
    }

    // 3. ★追加★ クラス名のみ指定の場合
    {
        FString ClassNameWithU = TypeString.StartsWith(TEXT("U")) ? TypeString : FString::Printf(TEXT("U%s"), *TypeString);
        
        for (TObjectIterator<UClass> It; It; ++It)
        {
            UClass* Class = *It;
            if (Class->GetName() == ClassNameWithU || Class->GetName() == TypeString)
            {
                if (Class->IsChildOf(UBTService::StaticClass()))
                {
                    return Class;
                }
            }
        }
    }

    // 4. Blueprint検索（既存のまま）
    // ...
}
```

### 4. 必要なインクルード追加

**ファイル先頭に追加（必要であれば）**:

```cpp
#include "UObject/UObjectIterator.h"
```

---

## ファイル構成

```
MCPGameProject/Plugins/SpirrowBridge/Source/SpirrowBridge/
└── Private/Commands/
    └── SpirrowBridgeAICommands_BTNodeHelpers.cpp  ← 修正
```

---

## 完了条件

- [ ] `GetBTTaskNodeClass()` にC++クラス検索を追加
- [ ] `GetBTDecoratorClass()` にC++クラス検索を追加
- [ ] `GetBTServiceClass()` にC++クラス検索を追加
- [ ] コンパイルが通る
- [ ] UE Editorで動作確認（別プロジェクトのカスタムBTTaskが検索できる）

---

## テスト手順

1. SpirrowBridgeプラグインをビルド
2. TrapxTrapCppプロジェクトを開く
3. 以下のMCPコマンドを実行:

```python
# クラス名のみ指定
add_bt_task_node(
    behavior_tree_name="BT_AIFighter",
    task_type="BTTask_ChaseTarget",
    path="/Game/TrapxTrap/AI/BehaviorTrees"
)

# フルパス指定
add_bt_task_node(
    behavior_tree_name="BT_AIFighter",
    task_type="/Script/TrapxTrapCpp.BTTask_ChaseTarget",
    path="/Game/TrapxTrap/AI/BehaviorTrees"
)
```

4. エラーなくノードが追加されることを確認

---

## 使用例（実装後）

```python
# TrapxTrapCppのカスタムタスク
add_bt_task_node(
    behavior_tree_name="BT_AIFighter",
    task_type="BTTask_ChaseTarget",
    node_name="Chase Target",
    path="/Game/TrapxTrap/AI/BehaviorTrees"
)

add_bt_task_node(
    behavior_tree_name="BT_AIFighter",
    task_type="BTTask_Shoot",
    node_name="Shoot",
    path="/Game/TrapxTrap/AI/BehaviorTrees"
)

# カスタムデコレータ
add_bt_decorator_node(
    behavior_tree_name="BT_AIFighter",
    decorator_type="BTDecorator_CheckHP",
    target_node_id="...",
    path="/Game/TrapxTrap/AI/BehaviorTrees"
)

# カスタムサービス
add_bt_service_node(
    behavior_tree_name="BT_AIFighter",
    service_type="BTService_UpdateBlackboard",
    target_node_id="...",
    path="/Game/TrapxTrap/AI/BehaviorTrees"
)
```

---

## 注意事項

- `TObjectIterator` は全クラスを走査するため、パフォーマンスに注意
- ただし、ノード追加は頻繁に行う操作ではないので許容範囲
- フルパス指定（`/Script/ModuleName.ClassName`）の方が確実で高速
- Blueprint検索は最後にフォールバックとして残す

---

## 参照ドキュメント

- `AGENTS.md` - コマンド追加チェックリスト
- `Docs/IMPLEMENTATION_SUMMARY.md` - C++ファイル構成
