# 更新履歴 (Changelog)

このファイルはspirrow-unrealwiseの詳細な更新履歴をアーカイブしています。

---

## 2026-01-07: Feature - Volume Actor Support in spawn_actor (v0.8.2)

**概要**: `spawn_actor`コマンドで8種類のVolumeアクターを生成可能に

**問題**:
- Volumeアクターは単純な`SpawnActor`だけではBrush Geometryが生成されない
- 結果としてDetailsパネルに「Brush Settings」が表示されず、機能しない

**解決策**:
- `UActorFactory::CreateBrushForVolumeActor`を使用して正しいBrush Geometryを生成
- `UCubeBuilder`でサイズを設定してからVolumeに適用

**対応したVolumeタイプ** (8種類):
- `NavMeshBoundsVolume` - NavMeshのビルド範囲
- `TriggerVolume` - トリガーイベント用
- `BlockingVolume` - コリジョンブロック用
- `KillZVolume` - 落下死亡用
- `PhysicsVolume` - 物理設定用
- `PostProcessVolume` - ポストプロセス用
- `AudioVolume` - オーディオ設定用
- `LightmassImportanceVolume` - ライトマス用

**新パラメータ**:
- `brush_size`: Volumeのサイズを明示的に指定 `[X, Y, Z]`
- 未指定時は `200 * scale` がデフォルト

**使用例**:
```python
# NavMeshボリュームを作成
spawn_actor(
    name="NavMesh_Arena",
    type="NavMeshBoundsVolume",
    location=[0, 0, 100],
    brush_size=[5000, 5000, 1000]
)

# トリガーボリュームを作成
spawn_actor(
    name="TrapZone",
    type="TriggerVolume",
    location=[500, 0, 50],
    brush_size=[300, 300, 200]
)
```

**変更ファイル**:
- `SpirrowBridgeEditorCommands.cpp` - Volumeアクター対応追加
- `SpirrowBridgeCommonUtils.h` - `InvalidActorType`エラーコード追加
- `editor_tools.py` - docstring更新、brush_size/scaleパラメータ追加

**追加include**:
- `NavMesh/NavMeshBoundsVolume.h`
- `GameFramework/KillZVolume.h`
- `Engine/TriggerVolume.h`
- `Engine/BlockingVolume.h`
- `GameFramework/PhysicsVolume.h`
- `Engine/PostProcessVolume.h`
- `Sound/AudioVolume.h`
- `Lightmass/LightmassImportanceVolume.h`
- `GameFramework/Volume.h`
- `ActorFactories/ActorFactory.h`
- `Builders/CubeBuilder.h`

---

## 2026-01-07: BugFix - Blackboard BaseClass Not Set (v0.8.1)

**概要**: `add_blackboard_key`の`base_class`パラメータが反映されない問題を修正

**問題**:
- `base_class="Actor"`を指定しても、Blackboardキーの BaseClass が `Object` のままだった
- 結果として MoveTo タスクで Object型キー（TargetActor等）が選択肢に表示されなかった

**原因**:
- `FindObject<UClass>(nullptr, *BaseClass)`で`"Actor"`という短い名前では検索できなかった
- 正しいパスは`/Script/Engine.Actor`

**修正内容**:
複数の検索方法を試行するように改善:
```cpp
// Method 1: 直接検索（フルパス用）
FoundClass = FindObject<UClass>(nullptr, *BaseClass);

// Method 2: /Script/Engine プレフィックス
FString EnginePath = FString::Printf(TEXT("/Script/Engine.%s"), *BaseClass);
FoundClass = FindObject<UClass>(nullptr, *EnginePath);

// Method 3: /Script/CoreUObject プレフィックス
FString CorePath = FString::Printf(TEXT("/Script/CoreUObject.%s"), *BaseClass);
FoundClass = FindObject<UClass>(nullptr, *CorePath);

// Method 4: StaticLoadClass フォールバック
FoundClass = StaticLoadClass(UObject::StaticClass(), nullptr, *ClassPath);
```

**変更ファイル**:
- `SpirrowBridgeAICommands_Blackboard.cpp` - BaseClass検索ロジック改善

**検証**:
- `add_blackboard_key(base_class="Actor")` → KeyType=Object, BaseClass=Actor ✅
- MoveTo タスクで TargetActor キーが選択可能に ✅
- `set_bt_node_property(property_name="BlackboardKey")` で設定反映 ✅

---

## 2026-01-07: Feature - Struct Property Support in SetObjectProperty

**概要**: `SetObjectProperty`に構造体プロパティ対応を追加

**対応構造体**:
- `FBlackboardKeySelector` - BTノードのBlackboardKey設定（文字列 or オブジェクト入力）
- `FVector` / `FVector2D` / `FRotator` - 座標・回転（配列 or オブジェクト入力）
- `FLinearColor` / `FColor` - 色（配列 or オブジェクト入力）
- `FAIDataProviderFloatValue` / `FAIDataProviderIntValue` / `FAIDataProviderBoolValue` - EQS用
- `FTransform` - トランスフォーム（オブジェクト入力）
- 汎用構造体 - リフレクションによるフィールド単位の設定

**追加プロパティタイプ**:
- `FNameProperty` - FName型
- `FDoubleProperty` - double型（UE5）

**使用例**:
```python
# BlackboardKey設定（文字列）
set_bt_node_property(node_id="...", property_name="BlackboardKey", property_value="TargetLocation")

# BlackboardKey設定（オブジェクト）
set_bt_node_property(node_id="...", property_name="BlackboardKey", property_value={"SelectedKeyName": "TargetActor"})

# Vector設定（配列）
set_actor_property(name="...", property_name="Location", property_value=[100, 200, 300])
```

**変更ファイル**:
- `SpirrowBridgeCommonUtils.h` - `SetStructPropertyValue`, `SetStructFieldValue` 宣言追加
- `SpirrowBridgeCommonUtils.cpp` - 構造体対応実装（約480行追加）

**既知の制限解消**:
- `set_eqs_test_property`でStruct型（FAIDataProviderFloatValue）が対応可能に

---

## 2026-01-07: BugFix - connect_bt_nodes Child Not Found

**概要**: `connect_bt_nodes`でRoot未接続ノードを検索できない問題を修正

**問題**:
- `add_bt_composite_node`で作成したノードを`connect_bt_nodes`でRootに接続しようとすると`Child node not found`エラー
- 原因: `FindBTNodeById`がRootNode以下のツリーのみを検索していた

**修正内容**:
- `PendingBTNodes`キャッシュを導入（作成済み・未接続ノードを一時保持）
- `HandleAddBTCompositeNode` / `HandleAddBTTaskNode`: ノード作成後にキャッシュに登録
- `FindBTNodeById`: まずキャッシュを検索、なければツリーを検索
- `HandleConnectBTNodes`: 接続成功後にキャッシュから削除

**変更ファイル (5ファイル)**:
- `SpirrowBridgeAICommands.h` - PendingBTNodes宣言追加
- `SpirrowBridgeAICommands.cpp` - static変数定義追加
- `AICommands_BTNodeCreation.cpp` - キャッシュ登録処理
- `AICommands_BTNodeHelpers.cpp` - キャッシュ検索処理
- `AICommands_BTNodeOperations.cpp` - キャッシュ削除処理

---

## 2026-01-06: Phase H - AIPerception & EQS

**概要**: AI感知システムとEnvironment Query System操作11ツール追加

**AIPerception (6ツール)**:
- `add_ai_perception_component` - AIControllerにPerceptionComponent追加
- `configure_sight_sense` - 視覚設定（距離/角度/アフィリエーション）
- `configure_hearing_sense` - 聴覚設定
- `configure_damage_sense` - ダメージ感知設定
- `set_perception_dominant_sense` - 優先センス設定
- `add_perception_stimuli_source` - 被検知側コンポーネント追加

**EQS (5ツール)**:
- `create_eqs_query` - EQS Query Asset作成
- `add_eqs_generator` - Generator追加（SimpleGrid/Donut/OnCircle/ActorsOfClass等）
- `add_eqs_test` - Test追加（Distance/Trace/Dot等）+ scoring_factor対応
- `set_eqs_test_property` - Testプロパティ設定（基本型のみ）
- `list_eqs_assets` - EQSアセット一覧

**技術詳細**:
- C++: AIPerceptionCommands (18KB) + EQSCommands (16KB)
- Python: perception_tools.py + eqs_tools.py
- テスト: test_phase_h.py (13テスト)

**既知の制限**:
- `set_eqs_test_property`でStruct型（FAIDataProviderFloatValue）は未対応
- → `add_eqs_test`の`scoring_factor`パラメータで代替可能

---

## 2026-01-06: Phase G - BehaviorTree Node Operations

**概要**: BTノードグラフをプログラマティックに構築する8ツール追加

**新規ツール**:
- `add_bt_composite_node` - Selector/Sequence/SimpleParallel追加
- `add_bt_task_node` - MoveTo/Wait等9タスク + カスタムBP対応
- `add_bt_decorator_node` - Blackboard/Cooldown等9デコレータ
- `add_bt_service_node` - DefaultFocus/RunEQS等サービス
- `connect_bt_nodes` - 親子接続、Root設定
- `set_bt_node_property` - リフレクション経由プロパティ設定
- `delete_bt_node` - ノード削除
- `list_bt_node_types` - 利用可能ノードタイプ一覧

**技術詳細**:
- C++ AICommands 6ファイル分割構成（1,805行）
- UE 5.6+ API互換性対応（Decorator格納方式、TryGetField変更）
- Python `tools/` フラット構造に統一

---

## 2026-01-05: Phase F - AI (BehaviorTree / Blackboard)

**概要**: AI開発に必須のBehaviorTree/Blackboard操作8ツール

**新規ツール**:
- `create_blackboard` - Blackboard Data Asset作成
- `add_blackboard_key` - キー追加（10タイプ対応）
- `remove_blackboard_key` / `list_blackboard_keys`
- `create_behavior_tree` - BehaviorTree Asset作成
- `set_behavior_tree_blackboard` / `get_behavior_tree_structure`
- `list_ai_assets` - AI関連アセット一覧

**技術詳細**:
- C++: SpirrowBridgeAICommands (674行)
- Python: ai_tools.py (455行)
- テスト: test_ai_tools.py (16テスト)

---

## 2026-01-03: Phase E - エラーハンドリング統一

**概要**: 全18 CommandsファイルにESpirrowErrorCode使用を統一

**追加エラーコード (12個)**:
- General: UnknownCommand, InvalidParameter, OperationFailed, SystemError
- Blueprint: GraphNotFound, NodeNotFound, ClassNotFound, InvalidOperation
- Actor: ComponentCreationFailed
- Config: ConfigKeyNotFound, FileWriteFailed, FileReadFailed

---

## 2026-01-03: Phase 0.6.6 - UMGWidgetCommands分割

**概要**: SpirrowBridgeUMGWidgetCommands.cpp (64KB) を3ファイルに分割

**分割構成**:
- UMGWidgetCoreCommands.cpp (7KB) - 3関数
- UMGWidgetBasicCommands.cpp (17KB) - 4関数
- UMGWidgetInteractiveCommands.cpp (30KB) - 7関数

**削減効果**: 最大64KB → 30KB (53%削減)

---

## 2026-01-03: Phase 0.6.5 - BlueprintCommands分割

**概要**: Blueprint系2ファイル（計163KB）を6ファイルに分割

**Blueprint系分割**:
- BlueprintCoreCommands.cpp (23KB)
- BlueprintComponentCommands.cpp (26KB)
- BlueprintPropertyCommands.cpp (21KB)

**BlueprintNode系分割**:
- BlueprintNodeCoreCommands.cpp (24KB)
- BlueprintNodeVariableCommands.cpp (14KB)
- BlueprintNodeControlFlowCommands.cpp (21KB)

**削減効果**: 最大95KB → 26KB (73%削減)

---

## それ以前の履歴

### Phase 0.6.0 - GAS対応
- GameplayTags / GameplayEffect / GameplayAbility ツール

### Phase 0.5.0 - UMG Widget
- Widget Blueprint操作（29ツール）
- アニメーション、変数・関数バインディング

### Phase 0.4.0 - Enhanced Input
- Input Action / Mapping Context
- Config操作ツール

### Phase 0.3.0 - Blueprint Node
- ノードグラフ操作
- RAG知識ベース統合
