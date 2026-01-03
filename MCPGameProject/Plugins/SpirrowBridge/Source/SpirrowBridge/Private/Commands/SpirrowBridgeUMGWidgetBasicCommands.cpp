#include "Commands/SpirrowBridgeUMGWidgetBasicCommands.h"
#include "Commands/SpirrowBridgeUMGWidgetCoreCommands.h"
#include "Commands/SpirrowBridgeCommonUtils.h"
#include "Editor.h"
#include "EditorAssetLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Engine/Texture2D.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet2/KismetEditorUtilities.h"

FSpirrowBridgeUMGWidgetBasicCommands::FSpirrowBridgeUMGWidgetBasicCommands()
{
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetBasicCommands::HandleCommand(const FString& CommandName, const TSharedPtr<FJsonObject>& Params)
{
	if (CommandName == TEXT("add_text_to_widget"))
	{
		return HandleAddTextToWidget(Params);
	}
	else if (CommandName == TEXT("add_text_block_to_widget"))
	{
		return HandleAddTextBlockToWidget(Params);
	}
	else if (CommandName == TEXT("add_image_to_widget"))
	{
		return HandleAddImageToWidget(Params);
	}
	else if (CommandName == TEXT("add_progressbar_to_widget"))
	{
		return HandleAddProgressBarToWidget(Params);
	}

	// Not handled by this class
	return nullptr;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetBasicCommands::HandleAddTextToWidget(const TSharedPtr<FJsonObject>& Params)
{
	// Validate required parameters
	FString WidgetName, TextName;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("widget_name"), WidgetName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("text_name"), TextName))
	{
		return Error;
	}

	// Get optional parameters
	FString Path, Text, AnchorStr;
	FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("path"), Path, TEXT("/Game/UI"));
	FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("text"), Text, TEXT("+"));
	FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("anchor"), AnchorStr, TEXT("Center"));

	double FontSizeDouble;
	FSpirrowBridgeCommonUtils::GetOptionalNumber(Params, TEXT("font_size"), FontSizeDouble, 32.0);
	int32 FontSize = static_cast<int32>(FontSizeDouble);

	// Get color array [R, G, B, A]
	FLinearColor Color = FSpirrowBridgeCommonUtils::GetLinearColorFromJson(Params, TEXT("color"), FLinearColor::White);

	// Get alignment [X, Y]
	FVector2D Alignment(0.5f, 0.5f);
	if (Params->HasField(TEXT("alignment")))
	{
		const TArray<TSharedPtr<FJsonValue>>* AlignmentArray;
		if (Params->TryGetArrayField(TEXT("alignment"), AlignmentArray) && AlignmentArray->Num() >= 2)
		{
			Alignment.X = (*AlignmentArray)[0]->AsNumber();
			Alignment.Y = (*AlignmentArray)[1]->AsNumber();
		}
	}

	FAnchors Anchors = FSpirrowBridgeUMGWidgetCoreCommands::ParseAnchorPreset(AnchorStr);

	// Validate and load Widget Blueprint
	UWidgetBlueprint* WidgetBP = nullptr;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateWidgetBlueprint(WidgetName, Path, WidgetBP))
	{
		return Error;
	}

	UWidgetTree* WidgetTree = WidgetBP->WidgetTree;
	if (!WidgetTree)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::WidgetTreeNotFound,
			TEXT("WidgetTree not found"));
	}

	// Get or create root Canvas Panel
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetTree->RootWidget = RootCanvas;
	}

	// Create TextBlock
	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), FName(*TextName));
	if (!TextBlock)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::WidgetCreationFailed,
			FString::Printf(TEXT("Failed to create TextBlock '%s'"), *TextName));
	}

	// Set text
	TextBlock->SetText(FText::FromString(Text));

	// Set font size
	FSlateFontInfo FontInfo = TextBlock->GetFont();
	FontInfo.Size = FontSize;
	TextBlock->SetFont(FontInfo);

	// Set color
	TextBlock->SetColorAndOpacity(FSlateColor(Color));

	// Add to Canvas Panel
	UCanvasPanelSlot* Slot = RootCanvas->AddChildToCanvas(TextBlock);
	if (Slot)
	{
		Slot->SetAnchors(Anchors);
		Slot->SetAlignment(Alignment);
		Slot->SetPosition(FVector2D(0, 0));
		Slot->SetAutoSize(true);
	}

	// Mark as modified and compile
	WidgetBP->Modify();
	WidgetBP->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	// Create success response
	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("widget"), WidgetName);
	ResultObj->SetStringField(TEXT("text_name"), TextName);
	ResultObj->SetStringField(TEXT("text"), Text);
	ResultObj->SetNumberField(TEXT("font_size"), FontSize);
	ResultObj->SetBoolField(TEXT("success"), true);
	return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetBasicCommands::HandleAddTextBlockToWidget(const TSharedPtr<FJsonObject>& Params)
{
	// Validate required parameters
	FString BlueprintName, WidgetName;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("blueprint_name"), BlueprintName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("widget_name"), WidgetName))
	{
		return Error;
	}

	// Find the Widget Blueprint (legacy path)
	FString FullPath = TEXT("/Game/Widgets/") + BlueprintName;
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(FullPath));
	if (!WidgetBlueprint)
	{
		TSharedPtr<FJsonObject> Details = MakeShared<FJsonObject>();
		Details->SetStringField(TEXT("blueprint_name"), BlueprintName);
		Details->SetStringField(TEXT("path"), TEXT("/Game/Widgets"));
		Details->SetStringField(TEXT("full_path"), FullPath);
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::WidgetNotFound,
			FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName),
			Details);
	}

	// Get optional parameters
	FString InitialText;
	FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("text"), InitialText, TEXT("New Text Block"));

	FVector2D Position(0.0f, 0.0f);
	if (Params->HasField(TEXT("position")))
	{
		const TArray<TSharedPtr<FJsonValue>>* PosArray;
		if (Params->TryGetArrayField(TEXT("position"), PosArray) && PosArray->Num() >= 2)
		{
			Position.X = (*PosArray)[0]->AsNumber();
			Position.Y = (*PosArray)[1]->AsNumber();
		}
	}

	// Create Text Block widget
	UTextBlock* TextBlock = WidgetBlueprint->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *WidgetName);
	if (!TextBlock)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::WidgetCreationFailed,
			FString::Printf(TEXT("Failed to create TextBlock '%s'"), *WidgetName));
	}

	// Set initial text
	TextBlock->SetText(FText::FromString(InitialText));

	// Add to canvas panel
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::CanvasPanelNotFound,
			TEXT("Root Canvas Panel not found"));
	}

	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(TextBlock);
	PanelSlot->SetPosition(Position);

	// Mark the package dirty and compile
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);

	// Create success response
	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("widget_name"), WidgetName);
	ResultObj->SetStringField(TEXT("text"), InitialText);
	ResultObj->SetBoolField(TEXT("success"), true);
	return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetBasicCommands::HandleAddImageToWidget(const TSharedPtr<FJsonObject>& Params)
{
	// Validate required parameters
	FString WidgetName, ImageName;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("widget_name"), WidgetName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("image_name"), ImageName))
	{
		return Error;
	}

	// Get optional parameters
	FString Path, TexturePath, AnchorStr;
	FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("path"), Path, TEXT("/Game/UI"));
	FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("texture_path"), TexturePath, TEXT(""));
	FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("anchor"), AnchorStr, TEXT("Center"));

	// Get size [Width, Height]
	FVector2D Size(32.0f, 32.0f);
	if (Params->HasField(TEXT("size")))
	{
		const TArray<TSharedPtr<FJsonValue>>* SizeArray;
		if (Params->TryGetArrayField(TEXT("size"), SizeArray) && SizeArray->Num() >= 2)
		{
			Size.X = (*SizeArray)[0]->AsNumber();
			Size.Y = (*SizeArray)[1]->AsNumber();
		}
	}

	// Get color tint [R, G, B, A]
	FLinearColor ColorTint = FSpirrowBridgeCommonUtils::GetLinearColorFromJson(Params, TEXT("color_tint"), FLinearColor::White);

	// Get alignment [X, Y]
	FVector2D Alignment(0.5f, 0.5f);
	if (Params->HasField(TEXT("alignment")))
	{
		const TArray<TSharedPtr<FJsonValue>>* AlignmentArray;
		if (Params->TryGetArrayField(TEXT("alignment"), AlignmentArray) && AlignmentArray->Num() >= 2)
		{
			Alignment.X = (*AlignmentArray)[0]->AsNumber();
			Alignment.Y = (*AlignmentArray)[1]->AsNumber();
		}
	}

	FAnchors Anchors = FSpirrowBridgeUMGWidgetCoreCommands::ParseAnchorPreset(AnchorStr);

	// Validate and load Widget Blueprint
	UWidgetBlueprint* WidgetBP = nullptr;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateWidgetBlueprint(WidgetName, Path, WidgetBP))
	{
		return Error;
	}

	UWidgetTree* WidgetTree = WidgetBP->WidgetTree;
	if (!WidgetTree)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::WidgetTreeNotFound,
			TEXT("WidgetTree not found"));
	}

	// Get or create root Canvas Panel
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetTree->RootWidget = RootCanvas;
	}

	// Create Image widget
	UImage* ImageWidget = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), FName(*ImageName));
	if (!ImageWidget)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::WidgetCreationFailed,
			FString::Printf(TEXT("Failed to create Image widget '%s'"), *ImageName));
	}

	// Load and set texture if path is provided
	if (!TexturePath.IsEmpty())
	{
		UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, *TexturePath);
		if (Texture)
		{
			ImageWidget->SetBrushFromTexture(Texture);
		}
	}

	// Set size
	ImageWidget->SetDesiredSizeOverride(Size);

	// Set color tint
	ImageWidget->SetColorAndOpacity(ColorTint);

	// Add to Canvas Panel
	UCanvasPanelSlot* Slot = RootCanvas->AddChildToCanvas(ImageWidget);
	if (Slot)
	{
		Slot->SetAnchors(Anchors);
		Slot->SetAlignment(Alignment);
		Slot->SetPosition(FVector2D(0, 0));
		Slot->SetSize(Size);
	}

	// Mark as modified and compile
	WidgetBP->Modify();
	WidgetBP->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	// Create success response
	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("widget"), WidgetName);
	ResultObj->SetStringField(TEXT("image_name"), ImageName);
	ResultObj->SetStringField(TEXT("texture_path"), TexturePath);
	ResultObj->SetBoolField(TEXT("success"), true);
	return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetBasicCommands::HandleAddProgressBarToWidget(const TSharedPtr<FJsonObject>& Params)
{
	// Validate required parameters
	FString WidgetName, ProgressBarName;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("widget_name"), WidgetName))
	{
		return Error;
	}
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("progressbar_name"), ProgressBarName))
	{
		return Error;
	}

	// Get optional parameters
	FString Path, AnchorStr;
	FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("path"), Path, TEXT("/Game/UI"));
	FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("anchor"), AnchorStr, TEXT("Center"));

	double Percent;
	FSpirrowBridgeCommonUtils::GetOptionalNumber(Params, TEXT("percent"), Percent, 0.0);

	// Get size [Width, Height]
	FVector2D Size(200.0f, 20.0f);
	if (Params->HasField(TEXT("size")))
	{
		const TArray<TSharedPtr<FJsonValue>>* SizeArray;
		if (Params->TryGetArrayField(TEXT("size"), SizeArray) && SizeArray->Num() >= 2)
		{
			Size.X = (*SizeArray)[0]->AsNumber();
			Size.Y = (*SizeArray)[1]->AsNumber();
		}
	}

	// Get colors
	FLinearColor FillColor = FSpirrowBridgeCommonUtils::GetLinearColorFromJson(Params, TEXT("fill_color"), FLinearColor(0.0f, 0.5f, 1.0f, 1.0f));
	FLinearColor BackgroundColor = FSpirrowBridgeCommonUtils::GetLinearColorFromJson(Params, TEXT("background_color"), FLinearColor(0.1f, 0.1f, 0.1f, 1.0f));

	// Get alignment [X, Y]
	FVector2D Alignment(0.5f, 0.5f);
	if (Params->HasField(TEXT("alignment")))
	{
		const TArray<TSharedPtr<FJsonValue>>* AlignmentArray;
		if (Params->TryGetArrayField(TEXT("alignment"), AlignmentArray) && AlignmentArray->Num() >= 2)
		{
			Alignment.X = (*AlignmentArray)[0]->AsNumber();
			Alignment.Y = (*AlignmentArray)[1]->AsNumber();
		}
	}

	FAnchors Anchors = FSpirrowBridgeUMGWidgetCoreCommands::ParseAnchorPreset(AnchorStr);

	// Validate and load Widget Blueprint
	UWidgetBlueprint* WidgetBP = nullptr;
	if (auto Error = FSpirrowBridgeCommonUtils::ValidateWidgetBlueprint(WidgetName, Path, WidgetBP))
	{
		return Error;
	}

	UWidgetTree* WidgetTree = WidgetBP->WidgetTree;
	if (!WidgetTree)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::WidgetTreeNotFound,
			TEXT("WidgetTree not found"));
	}

	// Get or create root Canvas Panel
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetTree->RootWidget = RootCanvas;
	}

	// Create ProgressBar widget
	UProgressBar* ProgressBarWidget = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), FName(*ProgressBarName));
	if (!ProgressBarWidget)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(
			ESpirrowErrorCode::WidgetCreationFailed,
			FString::Printf(TEXT("Failed to create ProgressBar widget '%s'"), *ProgressBarName));
	}

	// Set percent
	ProgressBarWidget->SetPercent(static_cast<float>(Percent));

	// Set fill color
	ProgressBarWidget->SetFillColorAndOpacity(FillColor);

	// Set background color
	FProgressBarStyle Style = ProgressBarWidget->GetWidgetStyle();
	Style.BackgroundImage.TintColor = FSlateColor(BackgroundColor);
	ProgressBarWidget->SetWidgetStyle(Style);

	// Add to Canvas Panel
	UCanvasPanelSlot* Slot = RootCanvas->AddChildToCanvas(ProgressBarWidget);
	if (Slot)
	{
		Slot->SetAnchors(Anchors);
		Slot->SetAlignment(Alignment);
		Slot->SetPosition(FVector2D(0, 0));
		Slot->SetSize(Size);
	}

	// Mark as modified and compile
	WidgetBP->Modify();
	WidgetBP->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	// Create success response
	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("widget"), WidgetName);
	ResultObj->SetStringField(TEXT("progressbar_name"), ProgressBarName);
	ResultObj->SetNumberField(TEXT("percent"), Percent);
	ResultObj->SetBoolField(TEXT("success"), true);
	return ResultObj;
}
