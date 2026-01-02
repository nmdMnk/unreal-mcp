#include "Commands/SpirrowBridgeUMGWidgetInteractiveCommands.h"
#include "Commands/SpirrowBridgeUMGWidgetCoreCommands.h"
#include "Commands/SpirrowBridgeCommonUtils.h"
#include "Editor.h"
#include "EditorAssetLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/SpinBox.h"
#include "Components/ScrollBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet2/KismetEditorUtilities.h"

FSpirrowBridgeUMGWidgetInteractiveCommands::FSpirrowBridgeUMGWidgetInteractiveCommands()
{
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetInteractiveCommands::HandleCommand(const FString& CommandName, const TSharedPtr<FJsonObject>& Params)
{
	if (CommandName == TEXT("add_button_to_widget"))
	{
		return HandleAddButtonToWidgetV2(Params);
	}
	else if (CommandName == TEXT("add_slider_to_widget"))
	{
		return HandleAddSliderToWidget(Params);
	}
	else if (CommandName == TEXT("add_checkbox_to_widget"))
	{
		return HandleAddCheckBoxToWidget(Params);
	}
	else if (CommandName == TEXT("add_combobox_to_widget"))
	{
		return HandleAddComboBoxToWidget(Params);
	}
	else if (CommandName == TEXT("add_editabletext_to_widget"))
	{
		return HandleAddEditableTextToWidget(Params);
	}
	else if (CommandName == TEXT("add_spinbox_to_widget"))
	{
		return HandleAddSpinBoxToWidget(Params);
	}
	else if (CommandName == TEXT("add_scrollbox_to_widget"))
	{
		return HandleAddScrollBoxToWidget(Params);
	}

	return nullptr;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetInteractiveCommands::HandleAddButtonToWidget(const TSharedPtr<FJsonObject>& Params)
{
	return HandleAddButtonToWidgetV2(Params);
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetInteractiveCommands::HandleAddButtonToWidgetV2(const TSharedPtr<FJsonObject>& Params)
{
	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	FString ButtonName;
	if (!Params->TryGetStringField(TEXT("button_name"), ButtonName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'button_name' parameter"));
	}

	FString Path = TEXT("/Game/UI");
	Params->TryGetStringField(TEXT("path"), Path);

	FString Text;
	Params->TryGetStringField(TEXT("text"), Text);

	int32 FontSize = 14;
	Params->TryGetNumberField(TEXT("font_size"), FontSize);

	FVector2D Size(200.0f, 50.0f);
	const TArray<TSharedPtr<FJsonValue>>* SizeArray;
	if (Params->TryGetArrayField(TEXT("size"), SizeArray) && SizeArray->Num() >= 2)
	{
		Size.X = (*SizeArray)[0]->AsNumber();
		Size.Y = (*SizeArray)[1]->AsNumber();
	}

	FLinearColor TextColor(1.0f, 1.0f, 1.0f, 1.0f);
	const TArray<TSharedPtr<FJsonValue>>* TextColorArray;
	if (Params->TryGetArrayField(TEXT("text_color"), TextColorArray) && TextColorArray->Num() >= 4)
	{
		TextColor.R = (*TextColorArray)[0]->AsNumber();
		TextColor.G = (*TextColorArray)[1]->AsNumber();
		TextColor.B = (*TextColorArray)[2]->AsNumber();
		TextColor.A = (*TextColorArray)[3]->AsNumber();
	}

	FLinearColor NormalColor(0.1f, 0.1f, 0.1f, 1.0f);
	const TArray<TSharedPtr<FJsonValue>>* NormalArray;
	if (Params->TryGetArrayField(TEXT("normal_color"), NormalArray) && NormalArray->Num() >= 4)
	{
		NormalColor.R = (*NormalArray)[0]->AsNumber();
		NormalColor.G = (*NormalArray)[1]->AsNumber();
		NormalColor.B = (*NormalArray)[2]->AsNumber();
		NormalColor.A = (*NormalArray)[3]->AsNumber();
	}

	FLinearColor HoveredColor(0.2f, 0.2f, 0.2f, 1.0f);
	const TArray<TSharedPtr<FJsonValue>>* HoveredArray;
	if (Params->TryGetArrayField(TEXT("hovered_color"), HoveredArray) && HoveredArray->Num() >= 4)
	{
		HoveredColor.R = (*HoveredArray)[0]->AsNumber();
		HoveredColor.G = (*HoveredArray)[1]->AsNumber();
		HoveredColor.B = (*HoveredArray)[2]->AsNumber();
		HoveredColor.A = (*HoveredArray)[3]->AsNumber();
	}

	FLinearColor PressedColor(0.05f, 0.05f, 0.05f, 1.0f);
	const TArray<TSharedPtr<FJsonValue>>* PressedArray;
	if (Params->TryGetArrayField(TEXT("pressed_color"), PressedArray) && PressedArray->Num() >= 4)
	{
		PressedColor.R = (*PressedArray)[0]->AsNumber();
		PressedColor.G = (*PressedArray)[1]->AsNumber();
		PressedColor.B = (*PressedArray)[2]->AsNumber();
		PressedColor.A = (*PressedArray)[3]->AsNumber();
	}

	FVector2D Alignment(0.5f, 0.5f);
	const TArray<TSharedPtr<FJsonValue>>* AlignArray;
	if (Params->TryGetArrayField(TEXT("alignment"), AlignArray) && AlignArray->Num() >= 2)
	{
		Alignment.X = (*AlignArray)[0]->AsNumber();
		Alignment.Y = (*AlignArray)[1]->AsNumber();
	}

	FString AnchorStr = TEXT("Center");
	Params->TryGetStringField(TEXT("anchor"), AnchorStr);
	FAnchors Anchors = FSpirrowBridgeUMGWidgetCoreCommands::ParseAnchorPreset(AnchorStr);

	FString AssetPath = Path + TEXT("/") + WidgetName + TEXT(".") + WidgetName;
	UWidgetBlueprint* WidgetBP = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(AssetPath));
	if (!WidgetBP)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget '%s' not found"), *WidgetName));
	}

	UWidgetTree* WidgetTree = WidgetBP->WidgetTree;
	if (!WidgetTree)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("WidgetTree not found"));
	}

	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetTree->RootWidget = RootCanvas;
	}

	UButton* ButtonWidget = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), FName(*ButtonName));
	if (!ButtonWidget)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create Button"));
	}

	FButtonStyle ButtonStyle = ButtonWidget->GetStyle();
	ButtonStyle.Normal.TintColor = FSlateColor(NormalColor);
	ButtonStyle.Hovered.TintColor = FSlateColor(HoveredColor);
	ButtonStyle.Pressed.TintColor = FSlateColor(PressedColor);
	ButtonWidget->SetStyle(ButtonStyle);

	if (!Text.IsEmpty())
	{
		FString TextBlockName = ButtonName + TEXT("_Text");
		UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), FName(*TextBlockName));
		if (TextBlock)
		{
			TextBlock->SetText(FText::FromString(Text));
			TextBlock->SetColorAndOpacity(FSlateColor(TextColor));
			FSlateFontInfo FontInfo = TextBlock->GetFont();
			FontInfo.Size = FontSize;
			TextBlock->SetFont(FontInfo);
			ButtonWidget->AddChild(TextBlock);
		}
	}

	UCanvasPanelSlot* Slot = RootCanvas->AddChildToCanvas(ButtonWidget);
	if (Slot)
	{
		Slot->SetAnchors(Anchors);
		Slot->SetAlignment(Alignment);
		Slot->SetPosition(FVector2D(0, 0));
		Slot->SetSize(Size);
	}

	WidgetBP->Modify();
	WidgetBP->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetBoolField(TEXT("success"), true);
	ResultObj->SetStringField(TEXT("widget_name"), WidgetName);
	ResultObj->SetStringField(TEXT("button_name"), ButtonName);
	ResultObj->SetStringField(TEXT("text"), Text);
	return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetInteractiveCommands::HandleAddSliderToWidget(const TSharedPtr<FJsonObject>& Params)
{
	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	FString SliderName;
	if (!Params->TryGetStringField(TEXT("slider_name"), SliderName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'slider_name' parameter"));
	}

	FString Path = TEXT("/Game/UI");
	Params->TryGetStringField(TEXT("path"), Path);

	float Value = 0.0f;
	Params->TryGetNumberField(TEXT("value"), Value);

	float MinValue = 0.0f;
	Params->TryGetNumberField(TEXT("min_value"), MinValue);

	float MaxValue = 1.0f;
	Params->TryGetNumberField(TEXT("max_value"), MaxValue);

	float StepSize = 0.0f;
	Params->TryGetNumberField(TEXT("step_size"), StepSize);

	FString OrientationStr = TEXT("Horizontal");
	Params->TryGetStringField(TEXT("orientation"), OrientationStr);
	EOrientation Orientation = (OrientationStr == TEXT("Vertical")) ? Orient_Vertical : Orient_Horizontal;

	FVector2D Size(200.0f, 20.0f);
	const TArray<TSharedPtr<FJsonValue>>* SizeArray;
	if (Params->TryGetArrayField(TEXT("size"), SizeArray) && SizeArray->Num() >= 2)
	{
		Size.X = (*SizeArray)[0]->AsNumber();
		Size.Y = (*SizeArray)[1]->AsNumber();
	}

	FLinearColor BarColor(0.2f, 0.2f, 0.2f, 1.0f);
	const TArray<TSharedPtr<FJsonValue>>* BarArray;
	if (Params->TryGetArrayField(TEXT("bar_color"), BarArray) && BarArray->Num() >= 4)
	{
		BarColor.R = (*BarArray)[0]->AsNumber();
		BarColor.G = (*BarArray)[1]->AsNumber();
		BarColor.B = (*BarArray)[2]->AsNumber();
		BarColor.A = (*BarArray)[3]->AsNumber();
	}

	FLinearColor HandleColor(1.0f, 1.0f, 1.0f, 1.0f);
	const TArray<TSharedPtr<FJsonValue>>* HandleArray;
	if (Params->TryGetArrayField(TEXT("handle_color"), HandleArray) && HandleArray->Num() >= 4)
	{
		HandleColor.R = (*HandleArray)[0]->AsNumber();
		HandleColor.G = (*HandleArray)[1]->AsNumber();
		HandleColor.B = (*HandleArray)[2]->AsNumber();
		HandleColor.A = (*HandleArray)[3]->AsNumber();
	}

	FVector2D Alignment(0.5f, 0.5f);
	const TArray<TSharedPtr<FJsonValue>>* AlignArray;
	if (Params->TryGetArrayField(TEXT("alignment"), AlignArray) && AlignArray->Num() >= 2)
	{
		Alignment.X = (*AlignArray)[0]->AsNumber();
		Alignment.Y = (*AlignArray)[1]->AsNumber();
	}

	FString AnchorStr = TEXT("Center");
	Params->TryGetStringField(TEXT("anchor"), AnchorStr);
	FAnchors Anchors = FSpirrowBridgeUMGWidgetCoreCommands::ParseAnchorPreset(AnchorStr);

	FString AssetPath = Path + TEXT("/") + WidgetName + TEXT(".") + WidgetName;
	UWidgetBlueprint* WidgetBP = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(AssetPath));
	if (!WidgetBP)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget '%s' not found"), *WidgetName));
	}

	UWidgetTree* WidgetTree = WidgetBP->WidgetTree;
	if (!WidgetTree)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("WidgetTree not found"));
	}

	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetTree->RootWidget = RootCanvas;
	}

	USlider* SliderWidget = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), FName(*SliderName));
	if (!SliderWidget)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create Slider"));
	}

	SliderWidget->SetValue(Value);
	SliderWidget->SetMinValue(MinValue);
	SliderWidget->SetMaxValue(MaxValue);
	SliderWidget->SetStepSize(StepSize);
	SliderWidget->SetOrientation(Orientation);

	FSliderStyle SliderStyle = SliderWidget->GetWidgetStyle();
	SliderStyle.NormalBarImage.TintColor = FSlateColor(BarColor);
	SliderStyle.HoveredBarImage.TintColor = FSlateColor(BarColor);
	SliderStyle.NormalThumbImage.TintColor = FSlateColor(HandleColor);
	SliderStyle.HoveredThumbImage.TintColor = FSlateColor(HandleColor);
	SliderWidget->SetWidgetStyle(SliderStyle);

	UCanvasPanelSlot* Slot = RootCanvas->AddChildToCanvas(SliderWidget);
	if (Slot)
	{
		Slot->SetAnchors(Anchors);
		Slot->SetAlignment(Alignment);
		Slot->SetPosition(FVector2D(0, 0));
		Slot->SetSize(Size);
	}

	WidgetBP->Modify();
	WidgetBP->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetBoolField(TEXT("success"), true);
	ResultObj->SetStringField(TEXT("widget_name"), WidgetName);
	ResultObj->SetStringField(TEXT("slider_name"), SliderName);
	ResultObj->SetNumberField(TEXT("value"), Value);
	return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetInteractiveCommands::HandleAddCheckBoxToWidget(const TSharedPtr<FJsonObject>& Params)
{
	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	FString CheckBoxName;
	if (!Params->TryGetStringField(TEXT("checkbox_name"), CheckBoxName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'checkbox_name' parameter"));
	}

	FString Path = TEXT("/Game/UI");
	Params->TryGetStringField(TEXT("path"), Path);

	bool bIsChecked = false;
	Params->TryGetBoolField(TEXT("is_checked"), bIsChecked);

	FString LabelText;
	Params->TryGetStringField(TEXT("label_text"), LabelText);

	FVector2D Alignment(0.5f, 0.5f);
	const TArray<TSharedPtr<FJsonValue>>* AlignArray;
	if (Params->TryGetArrayField(TEXT("alignment"), AlignArray) && AlignArray->Num() >= 2)
	{
		Alignment.X = (*AlignArray)[0]->AsNumber();
		Alignment.Y = (*AlignArray)[1]->AsNumber();
	}

	FString AnchorStr = TEXT("Center");
	Params->TryGetStringField(TEXT("anchor"), AnchorStr);
	FAnchors Anchors = FSpirrowBridgeUMGWidgetCoreCommands::ParseAnchorPreset(AnchorStr);

	FString AssetPath = Path + TEXT("/") + WidgetName + TEXT(".") + WidgetName;
	UWidgetBlueprint* WidgetBP = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(AssetPath));
	if (!WidgetBP)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget '%s' not found"), *WidgetName));
	}

	UWidgetTree* WidgetTree = WidgetBP->WidgetTree;
	if (!WidgetTree)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("WidgetTree not found"));
	}

	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetTree->RootWidget = RootCanvas;
	}

	UCheckBox* CheckBoxWidget = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), FName(*CheckBoxName));
	if (!CheckBoxWidget)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create CheckBox"));
	}

	CheckBoxWidget->SetIsChecked(bIsChecked);

	UWidget* WidgetToAdd = CheckBoxWidget;

	if (!LabelText.IsEmpty())
	{
		FString ContainerName = CheckBoxName + TEXT("_Container");
		UHorizontalBox* Container = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), FName(*ContainerName));
		Container->AddChild(CheckBoxWidget);

		FString LabelName = CheckBoxName + TEXT("_Label");
		UTextBlock* LabelWidget = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), FName(*LabelName));
		if (LabelWidget)
		{
			LabelWidget->SetText(FText::FromString(LabelText));
			Container->AddChild(LabelWidget);
			if (UHorizontalBoxSlot* LabelSlot = Cast<UHorizontalBoxSlot>(LabelWidget->Slot))
			{
				LabelSlot->SetPadding(FMargin(8.0f, 0.0f, 0.0f, 0.0f));
				LabelSlot->SetVerticalAlignment(VAlign_Center);
			}
		}
		WidgetToAdd = Container;
	}

	UCanvasPanelSlot* Slot = RootCanvas->AddChildToCanvas(WidgetToAdd);
	if (Slot)
	{
		Slot->SetAnchors(Anchors);
		Slot->SetAlignment(Alignment);
		Slot->SetPosition(FVector2D(0, 0));
		Slot->SetAutoSize(true);
	}

	WidgetBP->Modify();
	WidgetBP->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetBoolField(TEXT("success"), true);
	ResultObj->SetStringField(TEXT("widget_name"), WidgetName);
	ResultObj->SetStringField(TEXT("checkbox_name"), CheckBoxName);
	ResultObj->SetBoolField(TEXT("is_checked"), bIsChecked);
	return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetInteractiveCommands::HandleAddComboBoxToWidget(const TSharedPtr<FJsonObject>& Params)
{
	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	FString ComboBoxName;
	if (!Params->TryGetStringField(TEXT("combobox_name"), ComboBoxName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'combobox_name' parameter"));
	}

	FString Path = TEXT("/Game/UI");
	Params->TryGetStringField(TEXT("path"), Path);

	TArray<FString> Options;
	const TArray<TSharedPtr<FJsonValue>>* OptionsArray;
	if (Params->TryGetArrayField(TEXT("options"), OptionsArray))
	{
		for (const TSharedPtr<FJsonValue>& Val : *OptionsArray)
		{
			Options.Add(Val->AsString());
		}
	}

	int32 SelectedIndex = 0;
	Params->TryGetNumberField(TEXT("selected_index"), SelectedIndex);

	FVector2D Size(200.0f, 40.0f);
	const TArray<TSharedPtr<FJsonValue>>* SizeArray;
	if (Params->TryGetArrayField(TEXT("size"), SizeArray) && SizeArray->Num() >= 2)
	{
		Size.X = (*SizeArray)[0]->AsNumber();
		Size.Y = (*SizeArray)[1]->AsNumber();
	}

	FVector2D Alignment(0.5f, 0.5f);
	const TArray<TSharedPtr<FJsonValue>>* AlignArray;
	if (Params->TryGetArrayField(TEXT("alignment"), AlignArray) && AlignArray->Num() >= 2)
	{
		Alignment.X = (*AlignArray)[0]->AsNumber();
		Alignment.Y = (*AlignArray)[1]->AsNumber();
	}

	FString AnchorStr = TEXT("Center");
	Params->TryGetStringField(TEXT("anchor"), AnchorStr);
	FAnchors Anchors = FSpirrowBridgeUMGWidgetCoreCommands::ParseAnchorPreset(AnchorStr);

	FString AssetPath = Path + TEXT("/") + WidgetName + TEXT(".") + WidgetName;
	UWidgetBlueprint* WidgetBP = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(AssetPath));
	if (!WidgetBP)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget '%s' not found"), *WidgetName));
	}

	UWidgetTree* WidgetTree = WidgetBP->WidgetTree;
	if (!WidgetTree)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("WidgetTree not found"));
	}

	UCanvasPanel* Canvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
	if (!Canvas)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	UComboBoxString* ComboBoxWidget = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), FName(*ComboBoxName));
	if (!ComboBoxWidget)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create ComboBox"));
	}

	for (const FString& Option : Options)
	{
		ComboBoxWidget->AddOption(Option);
	}

	if (SelectedIndex >= 0 && SelectedIndex < Options.Num())
	{
		ComboBoxWidget->SetSelectedIndex(SelectedIndex);
	}

	UCanvasPanelSlot* CanvasSlot = Canvas->AddChildToCanvas(ComboBoxWidget);
	if (CanvasSlot)
	{
		CanvasSlot->SetAnchors(Anchors);
		CanvasSlot->SetAlignment(Alignment);
		CanvasSlot->SetAutoSize(false);
		CanvasSlot->SetSize(Size);
	}

	WidgetBP->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetBoolField(TEXT("success"), true);
	ResultObj->SetStringField(TEXT("widget_name"), WidgetName);
	ResultObj->SetStringField(TEXT("combobox_name"), ComboBoxName);
	ResultObj->SetNumberField(TEXT("option_count"), Options.Num());
	return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetInteractiveCommands::HandleAddEditableTextToWidget(const TSharedPtr<FJsonObject>& Params)
{
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

	FString Path = TEXT("/Game/UI");
	Params->TryGetStringField(TEXT("path"), Path);

	FString HintText;
	Params->TryGetStringField(TEXT("hint_text"), HintText);

	bool bIsPassword = false;
	Params->TryGetBoolField(TEXT("is_password"), bIsPassword);

	bool bIsMultiline = false;
	Params->TryGetBoolField(TEXT("is_multiline"), bIsMultiline);

	FVector2D Size(200.0f, 40.0f);
	const TArray<TSharedPtr<FJsonValue>>* SizeArray;
	if (Params->TryGetArrayField(TEXT("size"), SizeArray) && SizeArray->Num() >= 2)
	{
		Size.X = (*SizeArray)[0]->AsNumber();
		Size.Y = (*SizeArray)[1]->AsNumber();
	}

	FVector2D Alignment(0.5f, 0.5f);
	const TArray<TSharedPtr<FJsonValue>>* AlignArray;
	if (Params->TryGetArrayField(TEXT("alignment"), AlignArray) && AlignArray->Num() >= 2)
	{
		Alignment.X = (*AlignArray)[0]->AsNumber();
		Alignment.Y = (*AlignArray)[1]->AsNumber();
	}

	FString AnchorStr = TEXT("Center");
	Params->TryGetStringField(TEXT("anchor"), AnchorStr);
	FAnchors Anchors = FSpirrowBridgeUMGWidgetCoreCommands::ParseAnchorPreset(AnchorStr);

	FString AssetPath = Path + TEXT("/") + WidgetName + TEXT(".") + WidgetName;
	UWidgetBlueprint* WidgetBP = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(AssetPath));
	if (!WidgetBP)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget '%s' not found"), *WidgetName));
	}

	UWidgetTree* WidgetTree = WidgetBP->WidgetTree;
	if (!WidgetTree)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("WidgetTree not found"));
	}

	UCanvasPanel* Canvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
	if (!Canvas)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	UWidget* TextWidget = nullptr;

	if (bIsMultiline)
	{
		UMultiLineEditableTextBox* MultiLineWidget = WidgetTree->ConstructWidget<UMultiLineEditableTextBox>(
			UMultiLineEditableTextBox::StaticClass(), FName(*TextName));
		if (!MultiLineWidget)
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create MultiLineEditableTextBox"));
		}
		MultiLineWidget->SetHintText(FText::FromString(HintText));
		TextWidget = MultiLineWidget;
	}
	else
	{
		UEditableTextBox* SingleLineWidget = WidgetTree->ConstructWidget<UEditableTextBox>(
			UEditableTextBox::StaticClass(), FName(*TextName));
		if (!SingleLineWidget)
		{
			return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create EditableTextBox"));
		}
		SingleLineWidget->SetHintText(FText::FromString(HintText));
		SingleLineWidget->SetIsPassword(bIsPassword);
		TextWidget = SingleLineWidget;
	}

	UCanvasPanelSlot* CanvasSlot = Canvas->AddChildToCanvas(TextWidget);
	if (CanvasSlot)
	{
		CanvasSlot->SetAnchors(Anchors);
		CanvasSlot->SetAlignment(Alignment);
		CanvasSlot->SetAutoSize(false);
		CanvasSlot->SetSize(Size);
	}

	WidgetBP->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetBoolField(TEXT("success"), true);
	ResultObj->SetStringField(TEXT("widget_name"), WidgetName);
	ResultObj->SetStringField(TEXT("text_name"), TextName);
	ResultObj->SetBoolField(TEXT("is_multiline"), bIsMultiline);
	return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetInteractiveCommands::HandleAddSpinBoxToWidget(const TSharedPtr<FJsonObject>& Params)
{
	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	FString SpinBoxName;
	if (!Params->TryGetStringField(TEXT("spinbox_name"), SpinBoxName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'spinbox_name' parameter"));
	}

	FString Path = TEXT("/Game/UI");
	Params->TryGetStringField(TEXT("path"), Path);

	double Value = 0.0;
	Params->TryGetNumberField(TEXT("value"), Value);

	double MinValue = 0.0;
	Params->TryGetNumberField(TEXT("min_value"), MinValue);

	double MaxValue = 100.0;
	Params->TryGetNumberField(TEXT("max_value"), MaxValue);

	double Delta = 1.0;
	Params->TryGetNumberField(TEXT("delta"), Delta);

	FVector2D Size(150.0f, 40.0f);
	const TArray<TSharedPtr<FJsonValue>>* SizeArray;
	if (Params->TryGetArrayField(TEXT("size"), SizeArray) && SizeArray->Num() >= 2)
	{
		Size.X = (*SizeArray)[0]->AsNumber();
		Size.Y = (*SizeArray)[1]->AsNumber();
	}

	FVector2D Alignment(0.5f, 0.5f);
	const TArray<TSharedPtr<FJsonValue>>* AlignArray;
	if (Params->TryGetArrayField(TEXT("alignment"), AlignArray) && AlignArray->Num() >= 2)
	{
		Alignment.X = (*AlignArray)[0]->AsNumber();
		Alignment.Y = (*AlignArray)[1]->AsNumber();
	}

	FString AnchorStr = TEXT("Center");
	Params->TryGetStringField(TEXT("anchor"), AnchorStr);
	FAnchors Anchors = FSpirrowBridgeUMGWidgetCoreCommands::ParseAnchorPreset(AnchorStr);

	FString AssetPath = Path + TEXT("/") + WidgetName + TEXT(".") + WidgetName;
	UWidgetBlueprint* WidgetBP = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(AssetPath));
	if (!WidgetBP)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget '%s' not found"), *WidgetName));
	}

	UWidgetTree* WidgetTree = WidgetBP->WidgetTree;
	if (!WidgetTree)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("WidgetTree not found"));
	}

	UCanvasPanel* Canvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
	if (!Canvas)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	USpinBox* SpinBoxWidget = WidgetTree->ConstructWidget<USpinBox>(USpinBox::StaticClass(), FName(*SpinBoxName));
	if (!SpinBoxWidget)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create SpinBox"));
	}

	SpinBoxWidget->SetValue(Value);
	SpinBoxWidget->SetMinValue(MinValue);
	SpinBoxWidget->SetMaxValue(MaxValue);
	SpinBoxWidget->SetDelta(Delta);

	UCanvasPanelSlot* CanvasSlot = Canvas->AddChildToCanvas(SpinBoxWidget);
	if (CanvasSlot)
	{
		CanvasSlot->SetAnchors(Anchors);
		CanvasSlot->SetAlignment(Alignment);
		CanvasSlot->SetAutoSize(false);
		CanvasSlot->SetSize(Size);
	}

	WidgetBP->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetBoolField(TEXT("success"), true);
	ResultObj->SetStringField(TEXT("widget_name"), WidgetName);
	ResultObj->SetStringField(TEXT("spinbox_name"), SpinBoxName);
	ResultObj->SetNumberField(TEXT("value"), Value);
	return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetInteractiveCommands::HandleAddScrollBoxToWidget(const TSharedPtr<FJsonObject>& Params)
{
	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	FString ScrollBoxName;
	if (!Params->TryGetStringField(TEXT("scrollbox_name"), ScrollBoxName))
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Missing 'scrollbox_name' parameter"));
	}

	FString Path = TEXT("/Game/UI");
	Params->TryGetStringField(TEXT("path"), Path);

	FString OrientationStr = TEXT("Vertical");
	Params->TryGetStringField(TEXT("orientation"), OrientationStr);

	FString ScrollBarVisibilityStr = TEXT("Visible");
	Params->TryGetStringField(TEXT("scroll_bar_visibility"), ScrollBarVisibilityStr);

	FVector2D Size(300.0f, 200.0f);
	const TArray<TSharedPtr<FJsonValue>>* SizeArray;
	if (Params->TryGetArrayField(TEXT("size"), SizeArray) && SizeArray->Num() >= 2)
	{
		Size.X = (*SizeArray)[0]->AsNumber();
		Size.Y = (*SizeArray)[1]->AsNumber();
	}

	FVector2D Alignment(0.5f, 0.5f);
	const TArray<TSharedPtr<FJsonValue>>* AlignArray;
	if (Params->TryGetArrayField(TEXT("alignment"), AlignArray) && AlignArray->Num() >= 2)
	{
		Alignment.X = (*AlignArray)[0]->AsNumber();
		Alignment.Y = (*AlignArray)[1]->AsNumber();
	}

	FString AnchorStr = TEXT("Center");
	Params->TryGetStringField(TEXT("anchor"), AnchorStr);
	FAnchors Anchors = FSpirrowBridgeUMGWidgetCoreCommands::ParseAnchorPreset(AnchorStr);

	FString AssetPath = Path + TEXT("/") + WidgetName + TEXT(".") + WidgetName;
	UWidgetBlueprint* WidgetBP = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(AssetPath));
	if (!WidgetBP)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget '%s' not found"), *WidgetName));
	}

	UWidgetTree* WidgetTree = WidgetBP->WidgetTree;
	if (!WidgetTree)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("WidgetTree not found"));
	}

	UCanvasPanel* Canvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
	if (!Canvas)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	UScrollBox* ScrollBoxWidget = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), FName(*ScrollBoxName));
	if (!ScrollBoxWidget)
	{
		return FSpirrowBridgeCommonUtils::CreateErrorResponse(TEXT("Failed to create ScrollBox"));
	}

	EOrientation Orientation = (OrientationStr == TEXT("Horizontal")) ? Orient_Horizontal : Orient_Vertical;
	ScrollBoxWidget->SetOrientation(Orientation);

	ESlateVisibility ScrollBarVisibility = ESlateVisibility::Visible;
	if (ScrollBarVisibilityStr == TEXT("Hidden"))
	{
		ScrollBarVisibility = ESlateVisibility::Hidden;
	}
	else if (ScrollBarVisibilityStr == TEXT("Collapsed"))
	{
		ScrollBarVisibility = ESlateVisibility::Collapsed;
	}
	ScrollBoxWidget->SetScrollBarVisibility(ScrollBarVisibility);

	UCanvasPanelSlot* CanvasSlot = Canvas->AddChildToCanvas(ScrollBoxWidget);
	if (CanvasSlot)
	{
		CanvasSlot->SetAnchors(Anchors);
		CanvasSlot->SetAlignment(Alignment);
		CanvasSlot->SetAutoSize(false);
		CanvasSlot->SetSize(Size);
	}

	WidgetBP->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBP);

	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetBoolField(TEXT("success"), true);
	ResultObj->SetStringField(TEXT("widget_name"), WidgetName);
	ResultObj->SetStringField(TEXT("scrollbox_name"), ScrollBoxName);
	ResultObj->SetStringField(TEXT("orientation"), OrientationStr);
	return ResultObj;
}
