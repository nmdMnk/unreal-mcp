#include "Commands/SpirrowBridgeProjectCommands.h"
#include "Commands/SpirrowBridgeCommonUtils.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputTriggers.h"
#include "InputModifiers.h"
#include "EnhancedInputSubsystems.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "EditorAssetLibrary.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "K2Node_Event.h"
#include "K2Node_CallFunction.h"
#include "K2Node_DynamicCast.h"
#include "EdGraphSchema_K2.h"

FSpirrowBridgeProjectCommands::FSpirrowBridgeProjectCommands()
{
}

TSharedPtr<FJsonObject> FSpirrowBridgeProjectCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("create_input_mapping"))
    {
        return HandleCreateInputMapping(Params);
    }
    else if (CommandType == TEXT("create_input_action"))
    {
        return HandleCreateInputAction(Params);
    }
    else if (CommandType == TEXT("create_input_mapping_context"))
    {
        return HandleCreateInputMappingContext(Params);
    }
    else if (CommandType == TEXT("add_action_to_mapping_context"))
    {
        return HandleAddActionToMappingContext(Params);
    }
    else if (CommandType == TEXT("delete_asset"))
    {
        return HandleDeleteAsset(Params);
    }
    else if (CommandType == TEXT("add_mapping_context_to_blueprint"))
    {
        return HandleAddMappingContextToBlueprint(Params);
    }
    else if (CommandType == TEXT("set_default_mapping_context"))
    {
        return HandleSetDefaultMappingContext(Params);
    }

    return FSpirrowBridgeCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown project command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FSpirrowBridgeProjectCommands::HandleCreateInputMapping(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString ActionName;
    if (!Params->TryGetStringField(TEXT("action_name"), ActionName))
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'action_name' parameter"));
    }

    FString Key;
    if (!Params->TryGetStringField(TEXT("key"), Key))
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'key' parameter"));
    }

    // Get the input settings
    UInputSettings* InputSettings = GetMutableDefault<UInputSettings>();
    if (!InputSettings)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to get input settings"));
    }

    // Create the input action mapping
    FInputActionKeyMapping ActionMapping;
    ActionMapping.ActionName = FName(*ActionName);
    ActionMapping.Key = FKey(*Key);

    // Add modifiers if provided
    if (Params->HasField(TEXT("shift")))
    {
        ActionMapping.bShift = Params->GetBoolField(TEXT("shift"));
    }
    if (Params->HasField(TEXT("ctrl")))
    {
        ActionMapping.bCtrl = Params->GetBoolField(TEXT("ctrl"));
    }
    if (Params->HasField(TEXT("alt")))
    {
        ActionMapping.bAlt = Params->GetBoolField(TEXT("alt"));
    }
    if (Params->HasField(TEXT("cmd")))
    {
        ActionMapping.bCmd = Params->GetBoolField(TEXT("cmd"));
    }

    // Add the mapping
    InputSettings->AddActionMapping(ActionMapping);
    InputSettings->SaveConfig();

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("action_name"), ActionName);
    ResultObj->SetStringField(TEXT("key"), Key);
    return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeProjectCommands::HandleCreateInputAction(const TSharedPtr<FJsonObject>& Params)
{
    FString ActionName;
    if (!Params->TryGetStringField(TEXT("action_name"), ActionName))
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'action_name' parameter"));
    }

    FString ValueType;
    if (!Params->TryGetStringField(TEXT("value_type"), ValueType))
    {
        ValueType = TEXT("Digital");
    }

    FString Path;
    if (!Params->TryGetStringField(TEXT("path"), Path))
    {
        Path = TEXT("/Game/Input");
    }

    // ValueType マッピング
    EInputActionValueType ActionValueType = EInputActionValueType::Boolean;
    if (ValueType == TEXT("Axis1D"))
    {
        ActionValueType = EInputActionValueType::Axis1D;
    }
    else if (ValueType == TEXT("Axis2D"))
    {
        ActionValueType = EInputActionValueType::Axis2D;
    }
    else if (ValueType == TEXT("Axis3D"))
    {
        ActionValueType = EInputActionValueType::Axis3D;
    }

    // パッケージパス作成
    FString PackagePath = FString::Printf(TEXT("%s/%s"), *Path, *ActionName);

    // 既存アセットチェック
    if (UEditorAssetLibrary::DoesAssetExist(PackagePath))
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Input Action already exists: %s"), *PackagePath));
    }

    UPackage* Package = CreatePackage(*PackagePath);

    if (!Package)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create package"));
    }

    // InputAction作成
    UInputAction* NewAction = NewObject<UInputAction>(Package, *ActionName, RF_Public | RF_Standalone);

    if (!NewAction)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create Input Action"));
    }

    NewAction->ValueType = ActionValueType;

    // アセット登録
    FAssetRegistryModule::AssetCreated(NewAction);
    NewAction->MarkPackageDirty();

    // 保存
    FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    UPackage::SavePackage(Package, NewAction, *PackageFileName, SaveArgs);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("path"), PackagePath);
    ResultObj->SetStringField(TEXT("value_type"), ValueType);
    return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeProjectCommands::HandleCreateInputMappingContext(const TSharedPtr<FJsonObject>& Params)
{
    FString ContextName;
    if (!Params->TryGetStringField(TEXT("context_name"), ContextName))
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'context_name' parameter"));
    }

    FString Path;
    if (!Params->TryGetStringField(TEXT("path"), Path))
    {
        Path = TEXT("/Game/Input");
    }

    // パッケージパス作成
    FString PackagePath = FString::Printf(TEXT("%s/%s"), *Path, *ContextName);

    // 既存アセットチェック
    if (UEditorAssetLibrary::DoesAssetExist(PackagePath))
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Input Mapping Context already exists: %s"), *PackagePath));
    }

    UPackage* Package = CreatePackage(*PackagePath);

    if (!Package)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create package"));
    }

    // InputMappingContext作成
    UInputMappingContext* NewContext = NewObject<UInputMappingContext>(Package, *ContextName, RF_Public | RF_Standalone);

    if (!NewContext)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create Input Mapping Context"));
    }

    FAssetRegistryModule::AssetCreated(NewContext);
    NewContext->MarkPackageDirty();

    // 保存
    FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    UPackage::SavePackage(Package, NewContext, *PackageFileName, SaveArgs);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("path"), PackagePath);
    return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeProjectCommands::HandleAddActionToMappingContext(const TSharedPtr<FJsonObject>& Params)
{
    FString ContextName;
    if (!Params->TryGetStringField(TEXT("context_name"), ContextName))
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'context_name' parameter"));
    }

    FString ActionName;
    if (!Params->TryGetStringField(TEXT("action_name"), ActionName))
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'action_name' parameter"));
    }

    FString KeyName;
    if (!Params->TryGetStringField(TEXT("key"), KeyName))
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'key' parameter"));
    }

    FString TriggerType;
    if (!Params->TryGetStringField(TEXT("trigger_type"), TriggerType))
    {
        TriggerType = TEXT("Down");
    }

    FString ContextPath;
    if (!Params->TryGetStringField(TEXT("context_path"), ContextPath))
    {
        ContextPath = TEXT("/Game/Input");
    }

    FString ActionPath;
    if (!Params->TryGetStringField(TEXT("action_path"), ActionPath))
    {
        ActionPath = TEXT("/Game/Input");
    }

    // IMCアセットをロード
    FString FullContextPath = FString::Printf(TEXT("%s/%s.%s"), *ContextPath, *ContextName, *ContextName);
    UInputMappingContext* Context = LoadObject<UInputMappingContext>(nullptr, *FullContextPath);

    if (!Context)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Input Mapping Context not found: %s"), *FullContextPath));
    }

    // IAアセットをロード
    FString FullActionPath = FString::Printf(TEXT("%s/%s.%s"), *ActionPath, *ActionName, *ActionName);
    UInputAction* Action = LoadObject<UInputAction>(nullptr, *FullActionPath);

    if (!Action)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Input Action not found: %s"), *FullActionPath));
    }

    // キーをパース
    FKey Key(*KeyName);
    if (!Key.IsValid())
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Invalid key: %s"), *KeyName));
    }

    // マッピング追加
    FEnhancedActionKeyMapping& Mapping = Context->MapKey(Action, Key);

    // トリガー設定
    if (TriggerType == TEXT("Hold"))
    {
        UInputTriggerHold* Trigger = NewObject<UInputTriggerHold>(Context);
        Mapping.Triggers.Add(Trigger);
    }
    else if (TriggerType == TEXT("Pressed"))
    {
        UInputTriggerPressed* Trigger = NewObject<UInputTriggerPressed>(Context);
        Mapping.Triggers.Add(Trigger);
    }
    else if (TriggerType == TEXT("Released"))
    {
        UInputTriggerReleased* Trigger = NewObject<UInputTriggerReleased>(Context);
        Mapping.Triggers.Add(Trigger);
    }
    else if (TriggerType == TEXT("HoldAndRelease"))
    {
        UInputTriggerHoldAndRelease* Trigger = NewObject<UInputTriggerHoldAndRelease>(Context);
        Mapping.Triggers.Add(Trigger);
    }
    // "Down" はデフォルト動作、トリガー追加不要

    // Modifiers処理
    const TArray<TSharedPtr<FJsonValue>>* ModifiersArray;
    if (Params->TryGetArrayField(TEXT("modifiers"), ModifiersArray))
    {
        for (const auto& ModValue : *ModifiersArray)
        {
            FString ModName = ModValue->AsString();

            if (ModName == TEXT("Negate"))
            {
                UInputModifierNegate* Mod = NewObject<UInputModifierNegate>(Context);
                Mapping.Modifiers.Add(Mod);
            }
            else if (ModName == TEXT("SwizzleYXZ"))
            {
                UInputModifierSwizzleAxis* Mod = NewObject<UInputModifierSwizzleAxis>(Context);
                Mod->Order = EInputAxisSwizzle::YXZ;
                Mapping.Modifiers.Add(Mod);
            }
            else if (ModName == TEXT("DeadZone"))
            {
                UInputModifierDeadZone* Mod = NewObject<UInputModifierDeadZone>(Context);
                Mapping.Modifiers.Add(Mod);
            }
        }
    }

    // 保存
    Context->MarkPackageDirty();
    FString PackagePath = FString::Printf(TEXT("%s/%s"), *ContextPath, *ContextName);
    FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    UPackage::SavePackage(Context->GetOutermost(), Context, *PackageFileName, SaveArgs);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("context"), ContextName);
    ResultObj->SetStringField(TEXT("action"), ActionName);
    ResultObj->SetStringField(TEXT("key"), KeyName);
    ResultObj->SetStringField(TEXT("trigger"), TriggerType);
    return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeProjectCommands::HandleDeleteAsset(const TSharedPtr<FJsonObject>& Params)
{
    FString AssetPath;
    if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'asset_path' parameter"));
    }

    // アセットが存在するか確認
    if (!UEditorAssetLibrary::DoesAssetExist(AssetPath))
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Asset not found: %s"), *AssetPath));
    }

    // アセット削除
    bool bDeleted = UEditorAssetLibrary::DeleteAsset(AssetPath);

    if (!bDeleted)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Failed to delete asset: %s"), *AssetPath));
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("deleted"), AssetPath);
    return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeProjectCommands::HandleAddMappingContextToBlueprint(const TSharedPtr<FJsonObject>& Params)
{
    // パラメータ取得
    FString BlueprintName, ContextName, ContextPath, Path;
    int32 Priority = 0;

    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name'"));
    if (!Params->TryGetStringField(TEXT("context_name"), ContextName))
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'context_name'"));

    Params->TryGetStringField(TEXT("context_path"), ContextPath);
    Params->TryGetStringField(TEXT("path"), Path);
    Params->TryGetNumberField(TEXT("priority"), Priority);

    if (ContextPath.IsEmpty()) ContextPath = TEXT("/Game/Input");
    if (Path.IsEmpty()) Path = TEXT("/Game/Blueprints");

    // Blueprintをロード
    UBlueprint* Blueprint = FSpirrowBridgeCommonUtils::FindBlueprint(BlueprintName, Path);
    if (!Blueprint)
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Blueprint not found: %s/%s"), *Path, *BlueprintName));

    // IMCをロード
    FString FullContextPath = FString::Printf(TEXT("%s/%s.%s"), *ContextPath, *ContextName, *ContextName);
    UInputMappingContext* MappingContext = LoadObject<UInputMappingContext>(nullptr, *FullContextPath);
    if (!MappingContext)
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("IMC not found: %s"), *FullContextPath));

    UEdGraph* EventGraph = FBlueprintEditorUtils::FindEventGraph(Blueprint);
    if (!EventGraph)
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Event graph not found"));

    // ノード配置用の開始位置
    int32 NodeX = 0;
    int32 NodeY = 0;

    TArray<FString> CreatedNodeIds;

    // 1. BeginPlayイベントを検索または作成
    UK2Node_Event* BeginPlayNode = nullptr;
    for (UEdGraphNode* Node : EventGraph->Nodes)
    {
        UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node);
        if (EventNode && EventNode->EventReference.GetMemberName() == FName("ReceiveBeginPlay"))
        {
            BeginPlayNode = EventNode;
            NodeX = BeginPlayNode->NodePosX + 300;
            NodeY = BeginPlayNode->NodePosY;
            break;
        }
    }

    if (!BeginPlayNode)
    {
        // BeginPlayノード作成
        // UE5.7ではAddDefaultEventNodeの引数が変更された
        int32 OutNodePosY = NodeY;
        BeginPlayNode = FKismetEditorUtilities::AddDefaultEventNode(
            Blueprint, EventGraph, FName("ReceiveBeginPlay"), AActor::StaticClass(), OutNodePosY);
        if (!BeginPlayNode)
            return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create BeginPlay event"));
        NodeX = BeginPlayNode->NodePosX + 300;
        NodeY = BeginPlayNode->NodePosY;
    }

    CreatedNodeIds.Add(BeginPlayNode->NodeGuid.ToString());

    // 2. GetController ノード
    UK2Node_CallFunction* GetControllerNode = NewObject<UK2Node_CallFunction>(EventGraph);
    UFunction* GetControllerFunc = APawn::StaticClass()->FindFunctionByName(TEXT("GetController"));
    if (GetControllerFunc)
    {
        GetControllerNode->SetFromFunction(GetControllerFunc);
    }
    else
    {
        GetControllerNode->FunctionReference.SetExternalMember(FName("GetController"), APawn::StaticClass());
    }
    GetControllerNode->NodePosX = NodeX;
    GetControllerNode->NodePosY = NodeY;
    EventGraph->AddNode(GetControllerNode, false, false);
    GetControllerNode->CreateNewGuid();
    GetControllerNode->PostPlacedNewNode();
    GetControllerNode->AllocateDefaultPins();
    CreatedNodeIds.Add(GetControllerNode->NodeGuid.ToString());
    NodeX += 250;

    // 3. CastToPlayerController ノード
    UK2Node_DynamicCast* CastNode = NewObject<UK2Node_DynamicCast>(EventGraph);
    CastNode->TargetType = APlayerController::StaticClass();
    CastNode->NodePosX = NodeX;
    CastNode->NodePosY = NodeY;
    EventGraph->AddNode(CastNode, false, false);
    CastNode->CreateNewGuid();
    CastNode->PostPlacedNewNode();
    CastNode->AllocateDefaultPins();
    CreatedNodeIds.Add(CastNode->NodeGuid.ToString());
    NodeX += 300;

    // 4. GetEnhancedInputLocalPlayerSubsystem ノード (静的関数)
    UK2Node_CallFunction* GetSubsystemNode = NewObject<UK2Node_CallFunction>(EventGraph);
    // ULocalPlayerSubsystem::GetLocalPlayerSubsystem を使用
    GetSubsystemNode->FunctionReference.SetExternalMember(
        FName("Get"),
        UEnhancedInputLocalPlayerSubsystem::StaticClass());
    GetSubsystemNode->NodePosX = NodeX;
    GetSubsystemNode->NodePosY = NodeY;
    EventGraph->AddNode(GetSubsystemNode, false, false);
    GetSubsystemNode->CreateNewGuid();
    GetSubsystemNode->PostPlacedNewNode();
    GetSubsystemNode->AllocateDefaultPins();
    CreatedNodeIds.Add(GetSubsystemNode->NodeGuid.ToString());
    NodeX += 300;

    // 5. AddMappingContext ノード
    UK2Node_CallFunction* AddContextNode = NewObject<UK2Node_CallFunction>(EventGraph);
    UFunction* AddContextFunc = UEnhancedInputLocalPlayerSubsystem::StaticClass()->FindFunctionByName(TEXT("AddMappingContext"));
    if (AddContextFunc)
    {
        AddContextNode->SetFromFunction(AddContextFunc);
    }
    else
    {
        AddContextNode->FunctionReference.SetExternalMember(
            FName("AddMappingContext"),
            UEnhancedInputLocalPlayerSubsystem::StaticClass());
    }
    AddContextNode->NodePosX = NodeX;
    AddContextNode->NodePosY = NodeY;
    EventGraph->AddNode(AddContextNode, false, false);
    AddContextNode->CreateNewGuid();
    AddContextNode->PostPlacedNewNode();
    AddContextNode->AllocateDefaultPins();
    CreatedNodeIds.Add(AddContextNode->NodeGuid.ToString());

    // ピン接続
    const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

    // BeginPlay exec -> GetController exec (GetControllerには exec pinがないため、直接Castへ)
    // BeginPlay -> Cast exec
    UEdGraphPin* BeginPlayExec = BeginPlayNode->FindPin(UEdGraphSchema_K2::PN_Then);
    UEdGraphPin* CastExec = CastNode->FindPin(UEdGraphSchema_K2::PN_Execute);
    if (BeginPlayExec && CastExec)
    {
        BeginPlayExec->MakeLinkTo(CastExec);
    }

    // GetController ReturnValue -> Cast Object
    UEdGraphPin* GetControllerReturn = GetControllerNode->FindPin(UEdGraphSchema_K2::PN_ReturnValue);
    UEdGraphPin* CastObject = CastNode->FindPin(TEXT("Object"));
    if (GetControllerReturn && CastObject)
    {
        GetControllerReturn->MakeLinkTo(CastObject);
    }

    // Cast exec -> AddMappingContext exec
    UEdGraphPin* CastThenExec = CastNode->FindPin(UEdGraphSchema_K2::PN_Then);
    UEdGraphPin* AddContextExec = AddContextNode->FindPin(UEdGraphSchema_K2::PN_Execute);
    if (CastThenExec && AddContextExec)
    {
        CastThenExec->MakeLinkTo(AddContextExec);
    }

    // Cast result -> GetSubsystem PlayerController (if available)
    UEdGraphPin* CastResult = CastNode->GetCastResultPin();
    UEdGraphPin* SubsystemPlayerController = GetSubsystemNode->FindPin(TEXT("PlayerController"));
    if (CastResult && SubsystemPlayerController)
    {
        CastResult->MakeLinkTo(SubsystemPlayerController);
    }

    // GetSubsystem Return -> AddMappingContext Target
    UEdGraphPin* SubsystemReturn = GetSubsystemNode->FindPin(UEdGraphSchema_K2::PN_ReturnValue);
    UEdGraphPin* AddContextTarget = AddContextNode->FindPin(UEdGraphSchema_K2::PN_Self);
    if (SubsystemReturn && AddContextTarget)
    {
        SubsystemReturn->MakeLinkTo(AddContextTarget);
    }

    // MappingContextピンにデフォルト値設定
    UEdGraphPin* MappingContextPin = AddContextNode->FindPin(TEXT("MappingContext"));
    if (MappingContextPin)
    {
        MappingContextPin->DefaultObject = MappingContext;
    }

    // Priorityピン設定
    UEdGraphPin* PriorityPin = AddContextNode->FindPin(TEXT("Priority"));
    if (PriorityPin)
    {
        PriorityPin->DefaultValue = FString::FromInt(Priority);
    }

    // Blueprint コンパイル
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    FKismetEditorUtilities::CompileBlueprint(Blueprint);

    // 結果
    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);

    TArray<TSharedPtr<FJsonValue>> NodeIdsArray;
    for (const FString& NodeId : CreatedNodeIds)
    {
        NodeIdsArray.Add(MakeShared<FJsonValueString>(NodeId));
    }
    Result->SetArrayField(TEXT("nodes"), NodeIdsArray);
    Result->SetStringField(TEXT("message"),
        FString::Printf(TEXT("Added %d nodes for AddMappingContext to %s"), CreatedNodeIds.Num(), *BlueprintName));

    return Result;
}

TSharedPtr<FJsonObject> FSpirrowBridgeProjectCommands::HandleSetDefaultMappingContext(const TSharedPtr<FJsonObject>& Params)
{
    // パラメータ取得
    FString BlueprintName, ContextName, ContextPath, Path;
    int32 Priority = 0;

    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name'"));
    if (!Params->TryGetStringField(TEXT("context_name"), ContextName))
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'context_name'"));

    Params->TryGetStringField(TEXT("context_path"), ContextPath);
    Params->TryGetStringField(TEXT("path"), Path);
    Params->TryGetNumberField(TEXT("priority"), Priority);

    if (ContextPath.IsEmpty()) ContextPath = TEXT("/Game/Input");
    if (Path.IsEmpty()) Path = TEXT("/Game/Blueprints");

    // Blueprintをロード
    UBlueprint* Blueprint = FSpirrowBridgeCommonUtils::FindBlueprint(BlueprintName, Path);
    if (!Blueprint)
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Blueprint not found: %s/%s"), *Path, *BlueprintName));

    // 親クラスを確認
    UClass* ParentClass = Blueprint->ParentClass;
    bool bIsPlayerController = ParentClass && ParentClass->IsChildOf(APlayerController::StaticClass());

    if (bIsPlayerController)
    {
        // PlayerControllerの場合：DefaultMappingContextsプロパティを設定
        // UE5.4+ではDefaultMappingContextsが利用可能
        // ただし、CDOへの直接設定は複雑なため、BeginPlay方式を使用
        // 将来的にはDefaultMappingContextsへの直接設定を実装
        return HandleAddMappingContextToBlueprint(Params);
    }
    else
    {
        // Character/Pawnの場合：BeginPlay方式を使用
        return HandleAddMappingContextToBlueprint(Params);
    }
} 