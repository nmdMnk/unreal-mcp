#include "Commands/SpirrowBridgeBlueprintPropertyCommands.h"
#include "Commands/SpirrowBridgeCommonUtils.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Controller.h"
#include "Blueprint/UserWidget.h"
#include "Animation/AnimInstance.h"
#include "UObject/UObjectIterator.h"

FSpirrowBridgeBlueprintPropertyCommands::FSpirrowBridgeBlueprintPropertyCommands()
{
}

TSharedPtr<FJsonObject> FSpirrowBridgeBlueprintPropertyCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("scan_project_classes"))
    {
        return HandleScanProjectClasses(Params);
    }
    else if (CommandType == TEXT("set_blueprint_class_array"))
    {
        return HandleSetBlueprintClassArray(Params);
    }
    else if (CommandType == TEXT("set_struct_array_property"))
    {
        return HandleSetStructArrayProperty(Params);
    }

    return nullptr;
}

TSharedPtr<FJsonObject> FSpirrowBridgeBlueprintPropertyCommands::HandleScanProjectClasses(const TSharedPtr<FJsonObject>& Params)
{
    // Get optional parameters
    FString ClassType, ParentClassFilter, ModuleFilter, PathFilter, BlueprintTypeFilter;
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("class_type"), ClassType, TEXT("all"));
    Params->TryGetStringField(TEXT("parent_class"), ParentClassFilter);
    Params->TryGetStringField(TEXT("module_filter"), ModuleFilter);
    Params->TryGetStringField(TEXT("path_filter"), PathFilter);
    Params->TryGetStringField(TEXT("blueprint_type"), BlueprintTypeFilter);

    bool bIncludeEngine, bExcludeReinst;
    FSpirrowBridgeCommonUtils::GetOptionalBool(Params, TEXT("include_engine"), bIncludeEngine, false);
    FSpirrowBridgeCommonUtils::GetOptionalBool(Params, TEXT("exclude_reinst"), bExcludeReinst, true);

    TArray<TSharedPtr<FJsonValue>> CppClassesArray;
    TArray<TSharedPtr<FJsonValue>> BlueprintsArray;

    // === Scan C++ classes ===
    if (ClassType == TEXT("all") || ClassType == TEXT("cpp"))
    {
        for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
        {
            UClass* TestClass = *ClassIt;
            if (!TestClass || !TestClass->IsChildOf(AActor::StaticClass()))
            {
                continue;
            }

            FString ClassPath = TestClass->GetPathName();
            FString ClassName = TestClass->GetName();

            if (!bIncludeEngine)
            {
                if (ClassPath.Contains(TEXT("/Script/Engine.")) ||
                    ClassPath.Contains(TEXT("/Script/CoreUObject.")) ||
                    ClassPath.Contains(TEXT("/Script/UMG.")) ||
                    ClassPath.Contains(TEXT("/Script/AIModule.")) ||
                    ClassPath.Contains(TEXT("/Script/NavigationSystem.")) ||
                    ClassPath.Contains(TEXT("/Script/PhysicsCore.")) ||
                    ClassPath.Contains(TEXT("/Script/EnhancedInput.")) ||
                    ClassPath.Contains(TEXT("/Script/InputCore.")))
                {
                    continue;
                }
            }

            if (ClassName.EndsWith(TEXT("_C")))
            {
                continue;
            }

            if (bExcludeReinst)
            {
                if (ClassName.StartsWith(TEXT("REINST_")) || ClassPath.Contains(TEXT("/Engine/Transient.")))
                {
                    continue;
                }
            }

            if (!ModuleFilter.IsEmpty())
            {
                if (!ClassPath.Contains(FString::Printf(TEXT("/Script/%s."), *ModuleFilter)))
                {
                    continue;
                }
            }

            if (!ParentClassFilter.IsEmpty())
            {
                UClass* ParentClass = TestClass->GetSuperClass();
                bool bMatchesParent = false;
                while (ParentClass)
                {
                    if (ParentClass->GetName() == ParentClassFilter ||
                        ParentClass->GetName() == TEXT("A") + ParentClassFilter ||
                        ParentClass->GetName() == ParentClassFilter.Mid(1))
                    {
                        bMatchesParent = true;
                        break;
                    }
                    ParentClass = ParentClass->GetSuperClass();
                }
                if (!bMatchesParent) continue;
            }

            FString ModuleName;
            if (ClassPath.Contains(TEXT("/Script/")))
            {
                int32 ScriptIdx = ClassPath.Find(TEXT("/Script/"));
                int32 DotIdx = ClassPath.Find(TEXT("."), ESearchCase::IgnoreCase, ESearchDir::FromStart, ScriptIdx + 8);
                if (DotIdx != INDEX_NONE)
                {
                    ModuleName = ClassPath.Mid(ScriptIdx + 8, DotIdx - ScriptIdx - 8);
                }
            }

            FString ParentClassName;
            if (TestClass->GetSuperClass())
            {
                ParentClassName = TestClass->GetSuperClass()->GetName();
            }

            TSharedPtr<FJsonObject> ClassObj = MakeShared<FJsonObject>();
            ClassObj->SetStringField(TEXT("name"), ClassName);
            ClassObj->SetStringField(TEXT("path"), ClassPath);
            ClassObj->SetStringField(TEXT("parent"), ParentClassName);
            ClassObj->SetStringField(TEXT("module"), ModuleName);

            CppClassesArray.Add(MakeShared<FJsonValueObject>(ClassObj));
        }
    }

    // === Scan Blueprint assets ===
    if (ClassType == TEXT("all") || ClassType == TEXT("blueprint"))
    {
        FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
        IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

        FARFilter Filter;
        Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
        Filter.bRecursiveClasses = true;
        Filter.bRecursivePaths = true;

        if (!PathFilter.IsEmpty())
        {
            Filter.PackagePaths.Add(FName(*PathFilter));
        }
        else
        {
            Filter.PackagePaths.Add(FName(TEXT("/Game")));
        }

        TArray<FAssetData> AssetList;
        AssetRegistry.GetAssets(Filter, AssetList);

        for (const FAssetData& Asset : AssetList)
        {
            UBlueprint* Blueprint = Cast<UBlueprint>(Asset.GetAsset());
            if (!Blueprint) continue;

            if (!ParentClassFilter.IsEmpty() && Blueprint->ParentClass)
            {
                UClass* ParentClass = Blueprint->ParentClass;
                bool bMatchesParent = false;
                while (ParentClass)
                {
                    if (ParentClass->GetName() == ParentClassFilter ||
                        ParentClass->GetName() == TEXT("A") + ParentClassFilter ||
                        ParentClass->GetName() == ParentClassFilter.Mid(1))
                    {
                        bMatchesParent = true;
                        break;
                    }
                    ParentClass = ParentClass->GetSuperClass();
                }
                if (!bMatchesParent) continue;
            }

            if (!BlueprintTypeFilter.IsEmpty() && Blueprint->ParentClass)
            {
                bool bMatchesType = false;
                UClass* ParentClass = Blueprint->ParentClass;

                if (BlueprintTypeFilter == TEXT("actor"))
                {
                    bMatchesType = ParentClass->IsChildOf(AActor::StaticClass()) &&
                                   !ParentClass->IsChildOf(UUserWidget::StaticClass());
                }
                else if (BlueprintTypeFilter == TEXT("widget"))
                {
                    bMatchesType = ParentClass->IsChildOf(UUserWidget::StaticClass());
                }
                else if (BlueprintTypeFilter == TEXT("anim"))
                {
                    bMatchesType = ParentClass->IsChildOf(UAnimInstance::StaticClass());
                }
                else if (BlueprintTypeFilter == TEXT("controlrig"))
                {
                    static UClass* ControlRigClass = FindObject<UClass>(nullptr, TEXT("/Script/ControlRig.ControlRig"));
                    if (ControlRigClass)
                    {
                        bMatchesType = ParentClass->IsChildOf(ControlRigClass);
                    }
                }
                else if (BlueprintTypeFilter == TEXT("interface"))
                {
                    bMatchesType = ParentClass->IsChildOf(UInterface::StaticClass()) ||
                                   Asset.AssetName.ToString().StartsWith(TEXT("BPI_"));
                }
                else if (BlueprintTypeFilter == TEXT("gamemode"))
                {
                    bMatchesType = ParentClass->IsChildOf(AGameModeBase::StaticClass());
                }
                else if (BlueprintTypeFilter == TEXT("controller"))
                {
                    bMatchesType = ParentClass->IsChildOf(AController::StaticClass());
                }
                else if (BlueprintTypeFilter == TEXT("character"))
                {
                    bMatchesType = ParentClass->IsChildOf(ACharacter::StaticClass());
                }
                else if (BlueprintTypeFilter == TEXT("pawn"))
                {
                    bMatchesType = ParentClass->IsChildOf(APawn::StaticClass());
                }

                if (!bMatchesType) continue;
            }

            FString ParentClassName;
            if (Blueprint->ParentClass)
            {
                ParentClassName = Blueprint->ParentClass->GetName();
            }

            TSharedPtr<FJsonObject> BPObj = MakeShared<FJsonObject>();
            BPObj->SetStringField(TEXT("name"), Asset.AssetName.ToString());
            BPObj->SetStringField(TEXT("path"), Asset.GetObjectPathString());
            BPObj->SetStringField(TEXT("parent"), ParentClassName);

            BlueprintsArray.Add(MakeShared<FJsonValueObject>(BPObj));
        }
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetArrayField(TEXT("cpp_classes"), CppClassesArray);
    ResultObj->SetArrayField(TEXT("blueprints"), BlueprintsArray);
    ResultObj->SetNumberField(TEXT("total_cpp"), CppClassesArray.Num());
    ResultObj->SetNumberField(TEXT("total_blueprints"), BlueprintsArray.Num());

    return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeBlueprintPropertyCommands::HandleSetBlueprintClassArray(const TSharedPtr<FJsonObject>& Params)
{
    // Validate required parameters
    FString BlueprintName, PropertyName;
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("blueprint_name"), BlueprintName))
    {
        return Error;
    }
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("property_name"), PropertyName))
    {
        return Error;
    }

    const TArray<TSharedPtr<FJsonValue>>* ClassPathsArray = nullptr;
    if (!Params->TryGetArrayField(TEXT("class_paths"), ClassPathsArray))
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::MissingRequiredParam,
            TEXT("Missing required parameter: class_paths"));
    }

    // Get optional parameters
    FString Path;
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("path"), Path, TEXT("/Game/Blueprints"));

    // Load Blueprint
    UBlueprint* Blueprint = nullptr;
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateBlueprint(BlueprintName, Path, Blueprint))
    {
        return Error;
    }

    // Get CDO
    UClass* BPClass = Blueprint->GeneratedClass;
    if (!BPClass)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::InvalidOperation,
            TEXT("Blueprint has no generated class"));
    }

    UObject* CDO = BPClass->GetDefaultObject();
    if (!CDO)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::InvalidOperation,
            TEXT("Failed to get CDO"));
    }

    // Find property
    FArrayProperty* ArrayProp = FindFProperty<FArrayProperty>(BPClass, *PropertyName);
    if (!ArrayProp)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::PropertyNotFound,
            FString::Printf(TEXT("Property not found: %s"), *PropertyName));
    }

    FClassProperty* ClassProp = CastField<FClassProperty>(ArrayProp->Inner);
    if (!ClassProp)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::InvalidParameter,
            FString::Printf(TEXT("Property %s is not a TSubclassOf array"), *PropertyName));
    }

    void* ArrayPtr = ArrayProp->ContainerPtrToValuePtr<void>(CDO);
    FScriptArrayHelper ArrayHelper(ArrayProp, ArrayPtr);

    ArrayHelper.EmptyValues();

    int32 AddedCount = 0;
    for (const TSharedPtr<FJsonValue>& ClassPathValue : *ClassPathsArray)
    {
        FString ClassPath = ClassPathValue->AsString();
        UClass* LoadedClass = LoadObject<UClass>(nullptr, *ClassPath);
        if (!LoadedClass) continue;

        int32 NewIndex = ArrayHelper.AddValue();
        UClass** ElementPtr = reinterpret_cast<UClass**>(ArrayHelper.GetRawPtr(NewIndex));
        *ElementPtr = LoadedClass;
        AddedCount++;
    }

    Blueprint->Modify();
    Blueprint->MarkPackageDirty();
    FKismetEditorUtilities::CompileBlueprint(Blueprint);

    TSharedPtr<FJsonObject> ResultJson = MakeShareable(new FJsonObject());
    ResultJson->SetBoolField(TEXT("success"), true);
    ResultJson->SetStringField(TEXT("property_name"), PropertyName);
    ResultJson->SetNumberField(TEXT("count"), AddedCount);

    return ResultJson;
}

TSharedPtr<FJsonObject> FSpirrowBridgeBlueprintPropertyCommands::HandleSetStructArrayProperty(const TSharedPtr<FJsonObject>& Params)
{
    // Validate required parameters
    FString BlueprintName, PropertyName;
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("blueprint_name"), BlueprintName))
    {
        return Error;
    }
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("property_name"), PropertyName))
    {
        return Error;
    }

    const TArray<TSharedPtr<FJsonValue>>* ValuesArray = nullptr;
    if (!Params->TryGetArrayField(TEXT("values"), ValuesArray))
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::MissingRequiredParam,
            TEXT("Missing required parameter: values"));
    }

    // Get optional parameters
    FString Path;
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("path"), Path, TEXT("/Game/Blueprints"));

    // Load Blueprint
    UBlueprint* Blueprint = nullptr;
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateBlueprint(BlueprintName, Path, Blueprint))
    {
        return Error;
    }

    UClass* BPClass = Blueprint->GeneratedClass;
    if (!BPClass)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::InvalidOperation,
            TEXT("Blueprint has no generated class"));
    }

    UObject* CDO = BPClass->GetDefaultObject();
    if (!CDO)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::InvalidOperation,
            TEXT("Failed to get CDO"));
    }

    FArrayProperty* ArrayProp = FindFProperty<FArrayProperty>(BPClass, *PropertyName);
    if (!ArrayProp)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::PropertyNotFound,
            FString::Printf(TEXT("Property not found: %s"), *PropertyName));
    }

    FStructProperty* StructProp = CastField<FStructProperty>(ArrayProp->Inner);
    if (!StructProp)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::InvalidParameter,
            FString::Printf(TEXT("Property %s is not a struct array"), *PropertyName));
    }

    UScriptStruct* StructType = StructProp->Struct;

    void* ArrayPtr = ArrayProp->ContainerPtrToValuePtr<void>(CDO);
    FScriptArrayHelper ArrayHelper(ArrayProp, ArrayPtr);

    ArrayHelper.EmptyValues();
    ArrayHelper.Resize(ValuesArray->Num());

    int32 SetCount = 0;
    TArray<FString> Errors;

    for (int32 i = 0; i < ValuesArray->Num(); ++i)
    {
        const TSharedPtr<FJsonValue>& ElementValue = (*ValuesArray)[i];
        if (ElementValue->Type != EJson::Object)
        {
            Errors.Add(FString::Printf(TEXT("Element %d is not an object"), i));
            continue;
        }

        const TSharedPtr<FJsonObject>& ElementObj = ElementValue->AsObject();
        void* ElementPtr = ArrayHelper.GetRawPtr(i);

        for (TFieldIterator<FProperty> PropIt(StructType); PropIt; ++PropIt)
        {
            FProperty* FieldProp = *PropIt;
            FString FieldName = FieldProp->GetName();

            if (!ElementObj->HasField(FieldName)) continue;

            TSharedPtr<FJsonValue> FieldValue = ElementObj->TryGetField(FieldName);
            if (!FieldValue.IsValid()) continue;

            void* FieldPtr = FieldProp->ContainerPtrToValuePtr<void>(ElementPtr);

            if (FClassProperty* ClassFieldProp = CastField<FClassProperty>(FieldProp))
            {
                FString ClassPath = FieldValue->AsString();
                UClass* LoadedClass = LoadObject<UClass>(nullptr, *ClassPath);
                if (LoadedClass)
                {
                    ClassFieldProp->SetObjectPropertyValue(FieldPtr, LoadedClass);
                }
                else
                {
                    Errors.Add(FString::Printf(TEXT("Element %d: Failed to load class for %s: %s"), i, *FieldName, *ClassPath));
                }
            }
            else if (FIntProperty* IntFieldProp = CastField<FIntProperty>(FieldProp))
            {
                IntFieldProp->SetPropertyValue(FieldPtr, static_cast<int32>(FieldValue->AsNumber()));
            }
            else if (FFloatProperty* FloatFieldProp = CastField<FFloatProperty>(FieldProp))
            {
                FloatFieldProp->SetPropertyValue(FieldPtr, static_cast<float>(FieldValue->AsNumber()));
            }
            else if (FDoubleProperty* DoubleFieldProp = CastField<FDoubleProperty>(FieldProp))
            {
                DoubleFieldProp->SetPropertyValue(FieldPtr, FieldValue->AsNumber());
            }
            else if (FBoolProperty* BoolFieldProp = CastField<FBoolProperty>(FieldProp))
            {
                BoolFieldProp->SetPropertyValue(FieldPtr, FieldValue->AsBool());
            }
            else if (FStrProperty* StrFieldProp = CastField<FStrProperty>(FieldProp))
            {
                StrFieldProp->SetPropertyValue(FieldPtr, FieldValue->AsString());
            }
            else if (FNameProperty* NameFieldProp = CastField<FNameProperty>(FieldProp))
            {
                NameFieldProp->SetPropertyValue(FieldPtr, FName(*FieldValue->AsString()));
            }
        }

        SetCount++;
    }

    Blueprint->Modify();
    Blueprint->MarkPackageDirty();
    FKismetEditorUtilities::CompileBlueprint(Blueprint);

    TSharedPtr<FJsonObject> ResultJson = MakeShareable(new FJsonObject());
    ResultJson->SetBoolField(TEXT("success"), Errors.Num() == 0);
    ResultJson->SetStringField(TEXT("property_name"), PropertyName);
    ResultJson->SetStringField(TEXT("struct_type"), StructType->GetName());
    ResultJson->SetNumberField(TEXT("count"), SetCount);

    if (Errors.Num() > 0)
    {
        TArray<TSharedPtr<FJsonValue>> ErrorsArray;
        for (const FString& Error : Errors)
        {
            ErrorsArray.Add(MakeShareable(new FJsonValueString(Error)));
        }
        ResultJson->SetArrayField(TEXT("errors"), ErrorsArray);
    }

    return ResultJson;
}
