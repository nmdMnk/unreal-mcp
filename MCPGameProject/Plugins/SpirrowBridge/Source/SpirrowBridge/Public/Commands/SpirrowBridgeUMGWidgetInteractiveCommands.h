#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include "Widgets/Layout/Anchors.h"

/**
 * Handles UMG interactive widget commands
 * Responsible for Button, Slider, CheckBox, ComboBox, EditableText, SpinBox, ScrollBox
 */
class SPIRROWBRIDGE_API FSpirrowBridgeUMGWidgetInteractiveCommands
{
public:
    FSpirrowBridgeUMGWidgetInteractiveCommands();

    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
    TSharedPtr<FJsonObject> HandleAddButtonToWidget(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddButtonToWidgetV2(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddSliderToWidget(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddCheckBoxToWidget(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddComboBoxToWidget(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddEditableTextToWidget(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddSpinBoxToWidget(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddScrollBoxToWidget(const TSharedPtr<FJsonObject>& Params);
};
