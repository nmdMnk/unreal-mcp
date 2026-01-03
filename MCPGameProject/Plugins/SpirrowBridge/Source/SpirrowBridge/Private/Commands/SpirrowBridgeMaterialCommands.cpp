#include "Commands/SpirrowBridgeMaterialCommands.h"
#include "Commands/SpirrowBridgeCommonUtils.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Factories/MaterialFactoryNew.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "FileHelpers.h"
#include "EditorAssetLibrary.h"

FSpirrowBridgeMaterialCommands::FSpirrowBridgeMaterialCommands()
{
}

TSharedPtr<FJsonObject> FSpirrowBridgeMaterialCommands::HandleCommand(
    const FString& CommandType,
    const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("create_simple_material"))
    {
        return HandleCreateSimpleMaterial(Params);
    }

    return FSpirrowBridgeCommonUtils::CreateErrorResponse(
        ESpirrowErrorCode::UnknownCommand,
        FString::Printf(TEXT("Unknown material command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FSpirrowBridgeMaterialCommands::HandleCreateSimpleMaterial(
    const TSharedPtr<FJsonObject>& Params)
{
    // Validate required parameters
    FString Name;
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("name"), Name))
    {
        return Error;
    }

    // Get optional parameters
    FString Path, ShadingModelStr, BlendModeStr;
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("path"), Path, TEXT("/Game/Materials"));
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("shading_model"), ShadingModelStr, TEXT("DefaultLit"));
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("blend_mode"), BlendModeStr, TEXT("Opaque"));

    bool bTwoSided;
    FSpirrowBridgeCommonUtils::GetOptionalBool(Params, TEXT("two_sided"), bTwoSided, false);

    double Opacity, EmissiveStrength;
    FSpirrowBridgeCommonUtils::GetOptionalNumber(Params, TEXT("opacity"), Opacity, 1.0);
    FSpirrowBridgeCommonUtils::GetOptionalNumber(Params, TEXT("emissive_strength"), EmissiveStrength, 1.0);

    // Parse colors
    FLinearColor BaseColor = FLinearColor::White;
    FLinearColor EmissiveColor = FLinearColor::Black;

    const TArray<TSharedPtr<FJsonValue>>* BaseColorArray;
    if (Params->TryGetArrayField(TEXT("base_color"), BaseColorArray) && BaseColorArray->Num() >= 3)
    {
        BaseColor = FLinearColor(
            (*BaseColorArray)[0]->AsNumber(),
            (*BaseColorArray)[1]->AsNumber(),
            (*BaseColorArray)[2]->AsNumber()
        );
    }

    const TArray<TSharedPtr<FJsonValue>>* EmissiveColorArray;
    if (Params->TryGetArrayField(TEXT("emissive_color"), EmissiveColorArray) && EmissiveColorArray->Num() >= 3)
    {
        EmissiveColor = FLinearColor(
            (*EmissiveColorArray)[0]->AsNumber(),
            (*EmissiveColorArray)[1]->AsNumber(),
            (*EmissiveColorArray)[2]->AsNumber()
        );
    }

    // Create package
    FString PackagePath = Path / Name;

    // Check if material already exists
    if (UEditorAssetLibrary::DoesAssetExist(PackagePath))
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::AssetAlreadyExists,
            FString::Printf(TEXT("Material already exists at: %s"), *PackagePath));
    }

    UPackage* Package = CreatePackage(*PackagePath);
    if (!Package)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::AssetCreationFailed,
            TEXT("Failed to create package"));
    }

    // Create material
    UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();
    UMaterial* Material = Cast<UMaterial>(MaterialFactory->FactoryCreateNew(
        UMaterial::StaticClass(),
        Package,
        *Name,
        RF_Public | RF_Standalone,
        nullptr,
        GWarn
    ));

    if (!Material)
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::AssetCreationFailed,
            TEXT("Failed to create material"));
    }

    // Set blend mode
    if (BlendModeStr == TEXT("Translucent"))
    {
        Material->BlendMode = BLEND_Translucent;
    }
    else if (BlendModeStr == TEXT("Masked"))
    {
        Material->BlendMode = BLEND_Masked;
    }
    else
    {
        Material->BlendMode = BLEND_Opaque;
    }

    // Set shading model
    if (ShadingModelStr == TEXT("Unlit"))
    {
        Material->SetShadingModel(MSM_Unlit);
    }

    // Set two sided
    Material->TwoSided = bTwoSided;

    // Create material expressions
    int32 NodePosX = -300;
    int32 NodePosY = 0;

    // For Unlit: use Emissive Color
    // For DefaultLit: use Base Color
    if (ShadingModelStr == TEXT("Unlit"))
    {
        // Emissive Color node
        UMaterialExpressionConstant3Vector* EmissiveNode = NewObject<UMaterialExpressionConstant3Vector>(Material);
        EmissiveNode->Constant = EmissiveColor;
        EmissiveNode->MaterialExpressionEditorX = NodePosX;
        EmissiveNode->MaterialExpressionEditorY = NodePosY;
        Material->GetExpressionCollection().AddExpression(EmissiveNode);
        Material->GetEditorOnlyData()->EmissiveColor.Connect(0, EmissiveNode);
    }
    else
    {
        // Base Color node
        UMaterialExpressionConstant3Vector* BaseColorNode = NewObject<UMaterialExpressionConstant3Vector>(Material);
        BaseColorNode->Constant = BaseColor;
        BaseColorNode->MaterialExpressionEditorX = NodePosX;
        BaseColorNode->MaterialExpressionEditorY = NodePosY;
        Material->GetExpressionCollection().AddExpression(BaseColorNode);
        Material->GetEditorOnlyData()->BaseColor.Connect(0, BaseColorNode);

        // Emissive if specified
        if (EmissiveColor != FLinearColor::Black)
        {
            UMaterialExpressionConstant3Vector* EmissiveNode = NewObject<UMaterialExpressionConstant3Vector>(Material);
            EmissiveNode->Constant = EmissiveColor * EmissiveStrength;
            EmissiveNode->MaterialExpressionEditorX = NodePosX;
            EmissiveNode->MaterialExpressionEditorY = NodePosY + 150;
            Material->GetExpressionCollection().AddExpression(EmissiveNode);
            Material->GetEditorOnlyData()->EmissiveColor.Connect(0, EmissiveNode);
        }
    }

    // Opacity for Translucent
    if (Material->BlendMode == BLEND_Translucent)
    {
        UMaterialExpressionConstant* OpacityNode = NewObject<UMaterialExpressionConstant>(Material);
        OpacityNode->R = Opacity;
        OpacityNode->MaterialExpressionEditorX = NodePosX;
        OpacityNode->MaterialExpressionEditorY = NodePosY + 300;
        Material->GetExpressionCollection().AddExpression(OpacityNode);
        Material->GetEditorOnlyData()->Opacity.Connect(0, OpacityNode);
    }

    // Compile and save
    Material->PreEditChange(nullptr);
    Material->PostEditChange();

    // Mark package dirty and notify asset registry
    Package->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(Material);

    // Save the package
    FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    UPackage::SavePackage(Package, Material, *PackageFileName, SaveArgs);

    // Create success response
    TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject());
    Response->SetBoolField(TEXT("success"), true);
    Response->SetStringField(TEXT("asset_path"), PackagePath);
    Response->SetStringField(TEXT("name"), Name);
    Response->SetStringField(TEXT("blend_mode"), BlendModeStr);
    Response->SetStringField(TEXT("shading_model"), ShadingModelStr);
    Response->SetBoolField(TEXT("two_sided"), bTwoSided);

    return Response;
}
