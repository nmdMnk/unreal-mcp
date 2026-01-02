#include "Commands/SpirrowBridgeUMGWidgetCoreCommands.h"
#include "Commands/SpirrowBridgeCommonUtils.h"
#include "Editor.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/UserWidget.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"

FSpirrowBridgeUMGWidgetCoreCommands::FSpirrowBridgeUMGWidgetCoreCommands()
{
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetCoreCommands::HandleCommand(const FString& CommandName, const TSharedPtr<FJsonObject>& Params)
{
	if (CommandName == TEXT("create_umg_widget_blueprint"))
	{
		return HandleCreateUMGWidgetBlueprint(Params);
	}
	else if (CommandName == TEXT("add_widget_to_viewport"))
	{
		return HandleAddWidgetToViewport(Params);
	}

	// Not handled by this class
	return nullptr;
}

FAnchors FSpirrowBridgeUMGWidgetCoreCommands::ParseAnchorPreset(const FString& AnchorStr)
{
	if (AnchorStr == TEXT("TopLeft"))
	{
		return FAnchors(0.0f, 0.0f, 0.0f, 0.0f);
	}
	else if (AnchorStr == TEXT("TopCenter"))
	{
		return FAnchors(0.5f, 0.0f, 0.5f, 0.0f);
	}
	else if (AnchorStr == TEXT("TopRight"))
	{
		return FAnchors(1.0f, 0.0f, 1.0f, 0.0f);
	}
	else if (AnchorStr == TEXT("MiddleLeft"))
	{
		return FAnchors(0.0f, 0.5f, 0.0f, 0.5f);
	}
	else if (AnchorStr == TEXT("Center"))
	{
		return FAnchors(0.5f, 0.5f, 0.5f, 0.5f);
	}
	else if (AnchorStr == TEXT("MiddleRight"))
	{
		return FAnchors(1.0f, 0.5f, 1.0f, 0.5f);
	}
	else if (AnchorStr == TEXT("BottomLeft"))
	{
		return FAnchors(0.0f, 1.0f, 0.0f, 1.0f);
	}
	else if (AnchorStr == TEXT("BottomCenter"))
	{
		return FAnchors(0.5f, 1.0f, 0.5f, 1.0f);
	}
	else if (AnchorStr == TEXT("BottomRight"))
	{
		return FAnchors(1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Default: Center
	return FAnchors(0.5f, 0.5f, 0.5f, 0.5f);
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetCoreCommands::HandleCreateUMGWidgetBlueprint(const TSharedPtr<FJsonObject>& Params)
{
	// Get required parameters - support both "widget_name" and "name" for compatibility
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("widget_name"), BlueprintName))
	{
		if (!Params->TryGetStringField(TEXT("name"), BlueprintName))
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
		}
	}

	// Get optional path parameter (default: /Game/UI)
	FString PackagePath = TEXT("/Game/UI");
	Params->TryGetStringField(TEXT("path"), PackagePath);

	// Ensure path ends without trailing slash
	if (PackagePath.EndsWith(TEXT("/")))
	{
		PackagePath = PackagePath.LeftChop(1);
	}

	// Get optional parent class (default: UserWidget)
	FString ParentClassName = TEXT("UserWidget");
	Params->TryGetStringField(TEXT("parent_class"), ParentClassName);

	// Find the parent class
	UClass* ParentClass = nullptr;
	if (ParentClassName == TEXT("UserWidget"))
	{
		ParentClass = UUserWidget::StaticClass();
	}
	else
	{
		ParentClass = FindFirstObject<UClass>(*ParentClassName, EFindFirstObjectOptions::None);
		if (!ParentClass)
		{
			FString FullParentPath = FString::Printf(TEXT("/Script/UMG.%s"), *ParentClassName);
			ParentClass = LoadObject<UClass>(nullptr, *FullParentPath);
		}
		if (!ParentClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("Parent class '%s' not found, using UserWidget"), *ParentClassName);
			ParentClass = UUserWidget::StaticClass();
		}
	}

	// Create the full asset path
	FString AssetName = BlueprintName;
	FString FullPath = PackagePath + TEXT("/") + AssetName;

	// Check if asset already exists
	if (UEditorAssetLibrary::DoesAssetExist(FullPath))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' already exists at '%s'"), *BlueprintName, *FullPath));
	}

	// Create package
	UPackage* Package = CreatePackage(*FullPath);
	if (!Package)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create package"));
	}

	// Create Widget Blueprint
	UBlueprint* NewBlueprint = FKismetEditorUtilities::CreateBlueprint(
		ParentClass,
		Package,
		FName(*AssetName),
		BPTYPE_Normal,
		UWidgetBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass(),
		FName("CreateUMGWidget")
	);

	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(NewBlueprint);
	if (!WidgetBlueprint)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create Widget Blueprint"));
	}

	// Add default Canvas Panel
	if (!WidgetBlueprint->WidgetTree->RootWidget)
	{
		UCanvasPanel* RootCanvas = WidgetBlueprint->WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
		WidgetBlueprint->WidgetTree->RootWidget = RootCanvas;
	}

	// Mark and save
	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(WidgetBlueprint);
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);

	FString PackageFileName = FPackageName::LongPackageNameToFilename(FullPath, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.Error = GError;
	SaveArgs.SaveFlags = SAVE_NoError;
	UPackage::SavePackage(Package, WidgetBlueprint, *PackageFileName, SaveArgs);

	// Create success response
	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetBoolField(TEXT("success"), true);
	ResultObj->SetStringField(TEXT("name"), BlueprintName);
	ResultObj->SetStringField(TEXT("path"), FullPath);
	ResultObj->SetStringField(TEXT("parent_class"), ParentClass->GetName());
	return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetCoreCommands::HandleAddWidgetToViewport(const TSharedPtr<FJsonObject>& Params)
{
	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	// Find the Widget Blueprint
	FString FullPath = TEXT("/Game/Widgets/") + BlueprintName;
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(FullPath));
	if (!WidgetBlueprint)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Get optional Z-order parameter
	int32 ZOrder = 0;
	Params->TryGetNumberField(TEXT("z_order"), ZOrder);

	UClass* WidgetClass = WidgetBlueprint->GeneratedClass;
	if (!WidgetClass)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to get widget class"));
	}

	// Create success response
	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("blueprint_name"), BlueprintName);
	ResultObj->SetStringField(TEXT("class_path"), WidgetClass->GetPathName());
	ResultObj->SetNumberField(TEXT("z_order"), ZOrder);
	ResultObj->SetStringField(TEXT("note"), TEXT("Widget class ready. Use CreateWidget and AddToViewport nodes in Blueprint to display in game."));
	return ResultObj;
}
