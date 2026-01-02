#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include "Widgets/Layout/Anchors.h"

// Forward declarations
class FSpirrowBridgeUMGWidgetCoreCommands;
class FSpirrowBridgeUMGWidgetBasicCommands;
class FSpirrowBridgeUMGWidgetInteractiveCommands;

/**
 * Handles UMG Widget creation commands (Router)
 * Delegates to specialized command handlers:
 * - CoreCommands: create_umg_widget_blueprint, add_widget_to_viewport
 * - BasicCommands: add_text_to_widget, add_image_to_widget, add_progressbar_to_widget
 * - InteractiveCommands: add_button_to_widget, add_slider_to_widget, add_checkbox_to_widget, etc.
 */
class SPIRROWBRIDGE_API FSpirrowBridgeUMGWidgetCommands
{
public:
    FSpirrowBridgeUMGWidgetCommands();

    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

    // Helper - delegates to CoreCommands
    static FAnchors ParseAnchorPreset(const FString& AnchorStr);

private:
    TSharedPtr<FSpirrowBridgeUMGWidgetCoreCommands> CoreCommands;
    TSharedPtr<FSpirrowBridgeUMGWidgetBasicCommands> BasicCommands;
    TSharedPtr<FSpirrowBridgeUMGWidgetInteractiveCommands> InteractiveCommands;
};
