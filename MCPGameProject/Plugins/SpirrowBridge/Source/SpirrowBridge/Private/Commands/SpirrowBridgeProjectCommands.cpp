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

    return FSpirrowBridgeCommonUtils::CreateErrorResponse(
        ESpirrowErrorCode::UnknownCommand,
        FString::Printf(TEXT("Unknown project command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FSpirrowBridgeProjectCommands::HandleCreateInputMapping(const TSharedPtr<FJsonObject>& Params)
{
    // Validate required parameters
    FString ActionName, Key;
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("action_name"), ActionName))
    {
        return Error;
    }
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("key"), Key))
    {
        return Error;
    }

    UInputSettings* InputSettings = GetMutableDefault<UInputSettings>();
    if (!InputSettings)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::SystemError,
            TEXT("Failed to get input settings"));
    }

    FInputActionKeyMapping ActionMapping;
    ActionMapping.ActionName = FName(*ActionName);
    ActionMapping.Key = FKey(*Key);

    bool bShift = false, bCtrl = false, bAlt = false, bCmd = false;
    FSpirrowBridgeCommonUtils::GetOptionalBool(Params, TEXT("shift"), bShift, false);
    FSpirrowBridgeCommonUtils::GetOptionalBool(Params, TEXT("ctrl"), bCtrl, false);
    FSpirrowBridgeCommonUtils::GetOptionalBool(Params, TEXT("alt"), bAlt, false);
    FSpirrowBridgeCommonUtils::GetOptionalBool(Params, TEXT("cmd"), bCmd, false);
    ActionMapping.bShift = bShift;
    ActionMapping.bCtrl = bCtrl;
    ActionMapping.bAlt = bAlt;
    ActionMapping.bCmd = bCmd;

    InputSettings->AddActionMapping(ActionMapping);
    InputSettings->SaveConfig();

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("action_name"), ActionName);
    ResultObj->SetStringField(TEXT("key"), Key);
    return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeProjectCommands::HandleCreateInputAction(const TSharedPtr<FJsonObject>& Params)
{
    FString ActionName;
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("action_name"), ActionName))
    {
        return Error;
    }

    FString ValueType, Path;
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("value_type"), ValueType, TEXT("Digital"));
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("path"), Path, TEXT("/Game/Input"));

    EInputActionValueType ActionValueType = EInputActionValueType::Boolean;
    if (ValueType == TEXT("Axis1D"))
        ActionValueType = EInputActionValueType::Axis1D;
    else if (ValueType == TEXT("Axis2D"))
        ActionValueType = EInputActionValueType::Axis2D;
    else if (ValueType == TEXT("Axis3D"))
        ActionValueType = EInputActionValueType::Axis3D;

    FString PackagePath = FString::Printf(TEXT("%s/%s"), *Path, *ActionName);

    if (UEditorAssetLibrary::DoesAssetExist(PackagePath))
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::AssetAlreadyExists,
            FString::Printf(TEXT("Input Action already exists: %s"), *PackagePath));
    }

    UPackage* Package = CreatePackage(*PackagePath);
    if (!Package)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::AssetCreationFailed,
            TEXT("Failed to create package"));
    }

    UInputAction* NewAction = NewObject<UInputAction>(Package, *ActionName, RF_Public | RF_Standalone);
    if (!NewAction)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::AssetCreationFailed,
            TEXT("Failed to create Input Action"));
    }

    NewAction->ValueType = ActionValueType;

    FAssetRegistryModule::AssetCreated(NewAction);
    NewAction->MarkPackageDirty();

    FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    UPackage::SavePackage(Package, NewAction, *PackageFileName, SaveArgs);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("path"), PackagePath);
    ResultObj->SetStringField(TEXT("value_type"), ValueType);
    return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeProjectCommands::HandleCreateInputMappingContext(const TSharedPtr<FJsonObject>& Params)
{
    FString ContextName;
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("context_name"), ContextName))
    {
        return Error;
    }

    FString Path;
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("path"), Path, TEXT("/Game/Input"));

    FString PackagePath = FString::Printf(TEXT("%s/%s"), *Path, *ContextName);

    if (UEditorAssetLibrary::DoesAssetExist(PackagePath))
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::AssetAlreadyExists,
            FString::Printf(TEXT("Input Mapping Context already exists: %s"), *PackagePath));
    }

    UPackage* Package = CreatePackage(*PackagePath);
    if (!Package)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::AssetCreationFailed,
            TEXT("Failed to create package"));
    }

    UInputMappingContext* NewContext = NewObject<UInputMappingContext>(Package, *ContextName, RF_Public | RF_Standalone);
    if (!NewContext)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::AssetCreationFailed,
            TEXT("Failed to create Input Mapping Context"));
    }

    FAssetRegistryModule::AssetCreated(NewContext);
    NewContext->MarkPackageDirty();

    FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    UPackage::SavePackage(Package, NewContext, *PackageFileName, SaveArgs);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("path"), PackagePath);
    return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeProjectCommands::HandleAddActionToMappingContext(const TSharedPtr<FJsonObject>& Params)
{
    FString ContextName, ActionName, KeyName;
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("context_name"), ContextName))
    {
        return Error;
    }
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("action_name"), ActionName))
    {
        return Error;
    }
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("key"), KeyName))
    {
        return Error;
    }

    FString TriggerType, ContextPath, ActionPath;
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("trigger_type"), TriggerType, TEXT("Down"));
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("context_path"), ContextPath, TEXT("/Game/Input"));
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("action_path"), ActionPath, TEXT("/Game/Input"));

    FString FullContextPath = FString::Printf(TEXT("%s/%s.%s"), *ContextPath, *ContextName, *ContextName);
    UInputMappingContext* Context = LoadObject<UInputMappingContext>(nullptr, *FullContextPath);
    if (!Context)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::AssetNotFound,
            FString::Printf(TEXT("Input Mapping Context not found: %s"), *FullContextPath));
    }

    FString FullActionPath = FString::Printf(TEXT("%s/%s.%s"), *ActionPath, *ActionName, *ActionName);
    UInputAction* Action = LoadObject<UInputAction>(nullptr, *FullActionPath);
    if (!Action)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::AssetNotFound,
            FString::Printf(TEXT("Input Action not found: %s"), *FullActionPath));
    }

    FKey Key(*KeyName);
    if (!Key.IsValid())
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::InvalidParameter,
            FString::Printf(TEXT("Invalid key: %s"), *KeyName));
    }

    FEnhancedActionKeyMapping& Mapping = Context->MapKey(Action, Key);

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

    Context->MarkPackageDirty();
    FString PackagePath = FString::Printf(TEXT("%s/%s"), *ContextPath, *ContextName);
    FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    UPackage::SavePackage(Context->GetOutermost(), Context, *PackageFileName, SaveArgs);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("context"), ContextName);
    ResultObj->SetStringField(TEXT("action"), ActionName);
    ResultObj->SetStringField(TEXT("key"), KeyName);
    ResultObj->SetStringField(TEXT("trigger"), TriggerType);
    return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeProjectCommands::HandleDeleteAsset(const TSharedPtr<FJsonObject>& Params)
{
    FString AssetPath;
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("asset_path"), AssetPath))
    {
        return Error;
    }

    if (!UEditorAssetLibrary::DoesAssetExist(AssetPath))
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::AssetNotFound,
            FString::Printf(TEXT("Asset not found: %s"), *AssetPath));
    }

    bool bDeleted = UEditorAssetLibrary::DeleteAsset(AssetPath);
    if (!bDeleted)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::OperationFailed,
            FString::Printf(TEXT("Failed to delete asset: %s"), *AssetPath));
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("deleted"), AssetPath);
    return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeProjectCommands::HandleAddMappingContextToBlueprint(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, ContextName;
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("blueprint_name"), BlueprintName))
    {
        return Error;
    }
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("context_name"), ContextName))
    {
        return Error;
    }

    FString ContextPath, Path;
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("context_path"), ContextPath, TEXT("/Game/Input"));
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("path"), Path, TEXT("/Game/Blueprints"));

    double PriorityDouble;
    FSpirrowBridgeCommonUtils::GetOptionalNumber(Params, TEXT("priority"), PriorityDouble, 0.0);
    int32 Priority = static_cast<int32>(PriorityDouble);

    UBlueprint* Blueprint = FSpirrowBridgeCommonUtils::FindBlueprint(BlueprintName, Path);
    if (!Blueprint)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::BlueprintNotFound,
            FString::Printf(TEXT("Blueprint not found: %s/%s"), *Path, *BlueprintName));
    }

    FString FullContextPath = FString::Printf(TEXT("%s/%s.%s"), *ContextPath, *ContextName, *ContextName);
    UInputMappingContext* MappingContext = LoadObject<UInputMappingContext>(nullptr, *FullContextPath);
    if (!MappingContext)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::AssetNotFound,
            FString::Printf(TEXT("IMC not found: %s"), *FullContextPath));
    }

    UEdGraph* EventGraph = FBlueprintEditorUtils::FindEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::GraphNotFound,
            TEXT("Event graph not found"));
    }

    int32 NodeX = 0;
    int32 NodeY = 0;
    TArray<FString> CreatedNodeIds;

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
        int32 OutNodePosY = NodeY;
        BeginPlayNode = FKismetEditorUtilities::AddDefaultEventNode(
            Blueprint, EventGraph, FName("ReceiveBeginPlay"), AActor::StaticClass(), OutNodePosY);
        if (!BeginPlayNode)
        {
            return FSpirrowBridgeCommonUtils::CreateErrorResponse(
                ESpirrowErrorCode::NodeCreationFailed,
                TEXT("Failed to create BeginPlay event"));
        }
        NodeX = BeginPlayNode->NodePosX + 300;
        NodeY = BeginPlayNode->NodePosY;
    }

    CreatedNodeIds.Add(BeginPlayNode->NodeGuid.ToString());

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

    UK2Node_CallFunction* GetSubsystemNode = NewObject<UK2Node_CallFunction>(EventGraph);
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

    const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

    UEdGraphPin* BeginPlayExec = BeginPlayNode->FindPin(UEdGraphSchema_K2::PN_Then);
    UEdGraphPin* CastExec = CastNode->FindPin(UEdGraphSchema_K2::PN_Execute);
    if (BeginPlayExec && CastExec)
    {
        BeginPlayExec->MakeLinkTo(CastExec);
    }

    UEdGraphPin* GetControllerReturn = GetControllerNode->FindPin(UEdGraphSchema_K2::PN_ReturnValue);
    UEdGraphPin* CastObject = CastNode->FindPin(TEXT("Object"));
    if (GetControllerReturn && CastObject)
    {
        GetControllerReturn->MakeLinkTo(CastObject);
    }

    UEdGraphPin* CastThenExec = CastNode->FindPin(UEdGraphSchema_K2::PN_Then);
    UEdGraphPin* AddContextExec = AddContextNode->FindPin(UEdGraphSchema_K2::PN_Execute);
    if (CastThenExec && AddContextExec)
    {
        CastThenExec->MakeLinkTo(AddContextExec);
    }

    UEdGraphPin* CastResult = CastNode->GetCastResultPin();
    UEdGraphPin* SubsystemPlayerController = GetSubsystemNode->FindPin(TEXT("PlayerController"));
    if (CastResult && SubsystemPlayerController)
    {
        CastResult->MakeLinkTo(SubsystemPlayerController);
    }

    UEdGraphPin* SubsystemReturn = GetSubsystemNode->FindPin(UEdGraphSchema_K2::PN_ReturnValue);
    UEdGraphPin* AddContextTarget = AddContextNode->FindPin(UEdGraphSchema_K2::PN_Self);
    if (SubsystemReturn && AddContextTarget)
    {
        SubsystemReturn->MakeLinkTo(AddContextTarget);
    }

    UEdGraphPin* MappingContextPin = AddContextNode->FindPin(TEXT("MappingContext"));
    if (MappingContextPin)
    {
        MappingContextPin->DefaultObject = MappingContext;
    }

    UEdGraphPin* PriorityPin = AddContextNode->FindPin(TEXT("Priority"));
    if (PriorityPin)
    {
        PriorityPin->DefaultValue = FString::FromInt(Priority);
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    FKismetEditorUtilities::CompileBlueprint(Blueprint);

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
    FString BlueprintName, ContextName;
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("blueprint_name"), BlueprintName))
    {
        return Error;
    }
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("context_name"), ContextName))
    {
        return Error;
    }

    FString ContextPath, Path;
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("context_path"), ContextPath, TEXT("/Game/Input"));
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("path"), Path, TEXT("/Game/Blueprints"));

    UBlueprint* Blueprint = FSpirrowBridgeCommonUtils::FindBlueprint(BlueprintName, Path);
    if (!Blueprint)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::BlueprintNotFound,
            FString::Printf(TEXT("Blueprint not found: %s/%s"), *Path, *BlueprintName));
    }

    return HandleAddMappingContextToBlueprint(Params);
}
