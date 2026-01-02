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
	// Get required parameters
	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	FString TextName;
	if (!Params->TryGetStringField(TEXT("text_name"), TextName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'text_name' parameter"));
	}

	// Get optional parameters
	FString Path = TEXT("/Game/UI");
	Params->TryGetStringField(TEXT("path"), Path);

	FString Text = TEXT("+");
	Params->TryGetStringField(TEXT("text"), Text);

	int32 FontSize = 32;
	if (Params->HasField(TEXT("font_size")))
	{
		FontSize = Params->GetNumberField(TEXT("font_size"));
	}

	// Get color array [R, G, B, A]
	FLinearColor Color(1.0f, 1.0f, 1.0f, 1.0f);
	if (Params->HasField(TEXT("color")))
	{
		const TArray<TSharedPtr<FJsonValue>>* ColorArray;
		if (Params->TryGetArrayField(TEXT("color"), ColorArray) && ColorArray->Num() >= 4)
		{
			Color.R = (*ColorArray)[0]->AsNumber();
			Color.G = (*ColorArray)[1]->AsNumber();
			Color.B = (*ColorArray)[2]->AsNumber();
			Color.A = (*ColorArray)[3]->AsNumber();
		}
	}

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

	// Get anchor
	FString AnchorStr = TEXT("Center");
	Params->TryGetStringField(TEXT("anchor"), AnchorStr);
	FAnchors Anchors = FSpirrowBridgeUMGWidgetCoreCommands::ParseAnchorPreset(AnchorStr);

	// Load Widget Blueprint
	FString AssetPath = Path + TEXT("/") + WidgetName + TEXT(".") + WidgetName;
	UWidgetBlueprint* WidgetBP = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(AssetPath));
	if (!WidgetBP)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found at path '%s'"), *WidgetName, *AssetPath));
	}

	UWidgetTree* WidgetTree = WidgetBP->WidgetTree;
	if (!WidgetTree)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("WidgetTree not found"));
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
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create TextBlock"));
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
	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Find the Widget Blueprint
	FString FullPath = TEXT("/Game/Widgets/") + BlueprintName;
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(FullPath));
	if (!WidgetBlueprint)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Get optional parameters
	FString InitialText = TEXT("New Text Block");
	Params->TryGetStringField(TEXT("text"), InitialText);

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
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create Text Block widget"));
	}

	// Set initial text
	TextBlock->SetText(FText::FromString(InitialText));

	// Add to canvas panel
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Root Canvas Panel not found"));
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
	return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetBasicCommands::HandleAddImageToWidget(const TSharedPtr<FJsonObject>& Params)
{
	// Get required parameters
	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	FString ImageName;
	if (!Params->TryGetStringField(TEXT("image_name"), ImageName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'image_name' parameter"));
	}

	// Get optional parameters
	FString Path = TEXT("/Game/UI");
	Params->TryGetStringField(TEXT("path"), Path);

	FString TexturePath;
	Params->TryGetStringField(TEXT("texture_path"), TexturePath);

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
	FLinearColor ColorTint(1.0f, 1.0f, 1.0f, 1.0f);
	if (Params->HasField(TEXT("color_tint")))
	{
		const TArray<TSharedPtr<FJsonValue>>* ColorArray;
		if (Params->TryGetArrayField(TEXT("color_tint"), ColorArray) && ColorArray->Num() >= 4)
		{
			ColorTint.R = (*ColorArray)[0]->AsNumber();
			ColorTint.G = (*ColorArray)[1]->AsNumber();
			ColorTint.B = (*ColorArray)[2]->AsNumber();
			ColorTint.A = (*ColorArray)[3]->AsNumber();
		}
	}

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

	// Get anchor
	FString AnchorStr = TEXT("Center");
	Params->TryGetStringField(TEXT("anchor"), AnchorStr);
	FAnchors Anchors = FSpirrowBridgeUMGWidgetCoreCommands::ParseAnchorPreset(AnchorStr);

	// Load Widget Blueprint
	FString AssetPath = Path + TEXT("/") + WidgetName + TEXT(".") + WidgetName;
	UWidgetBlueprint* WidgetBP = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(AssetPath));
	if (!WidgetBP)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found at path '%s'"), *WidgetName, *AssetPath));
	}

	UWidgetTree* WidgetTree = WidgetBP->WidgetTree;
	if (!WidgetTree)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("WidgetTree not found"));
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
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create Image widget"));
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
	// Get required parameters
	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	FString ProgressBarName;
	if (!Params->TryGetStringField(TEXT("progressbar_name"), ProgressBarName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'progressbar_name' parameter"));
	}

	// Get optional parameters
	FString Path = TEXT("/Game/UI");
	Params->TryGetStringField(TEXT("path"), Path);

	float Percent = 0.0f;
	if (Params->HasField(TEXT("percent")))
	{
		Percent = Params->GetNumberField(TEXT("percent"));
	}

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

	// Get fill color [R, G, B, A]
	FLinearColor FillColor(0.0f, 0.5f, 1.0f, 1.0f);
	if (Params->HasField(TEXT("fill_color")))
	{
		const TArray<TSharedPtr<FJsonValue>>* ColorArray;
		if (Params->TryGetArrayField(TEXT("fill_color"), ColorArray) && ColorArray->Num() >= 4)
		{
			FillColor.R = (*ColorArray)[0]->AsNumber();
			FillColor.G = (*ColorArray)[1]->AsNumber();
			FillColor.B = (*ColorArray)[2]->AsNumber();
			FillColor.A = (*ColorArray)[3]->AsNumber();
		}
	}

	// Get background color [R, G, B, A]
	FLinearColor BackgroundColor(0.1f, 0.1f, 0.1f, 1.0f);
	if (Params->HasField(TEXT("background_color")))
	{
		const TArray<TSharedPtr<FJsonValue>>* ColorArray;
		if (Params->TryGetArrayField(TEXT("background_color"), ColorArray) && ColorArray->Num() >= 4)
		{
			BackgroundColor.R = (*ColorArray)[0]->AsNumber();
			BackgroundColor.G = (*ColorArray)[1]->AsNumber();
			BackgroundColor.B = (*ColorArray)[2]->AsNumber();
			BackgroundColor.A = (*ColorArray)[3]->AsNumber();
		}
	}

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

	// Get anchor
	FString AnchorStr = TEXT("Center");
	Params->TryGetStringField(TEXT("anchor"), AnchorStr);
	FAnchors Anchors = FSpirrowBridgeUMGWidgetCoreCommands::ParseAnchorPreset(AnchorStr);

	// Load Widget Blueprint
	FString AssetPath = Path + TEXT("/") + WidgetName + TEXT(".") + WidgetName;
	UWidgetBlueprint* WidgetBP = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(AssetPath));
	if (!WidgetBP)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found at path '%s'"), *WidgetName, *AssetPath));
	}

	UWidgetTree* WidgetTree = WidgetBP->WidgetTree;
	if (!WidgetTree)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("WidgetTree not found"));
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
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create ProgressBar widget"));
	}

	// Set percent
	ProgressBarWidget->SetPercent(Percent);

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
