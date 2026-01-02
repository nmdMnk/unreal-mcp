#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include "Widgets/Layout/Anchors.h"

/**
 * Handles UMG Widget core commands
 * Responsible for widget creation, viewport management, and utilities
 */
class SPIRROWBRIDGE_API FSpirrowBridgeUMGWidgetCoreCommands
{
public:
    FSpirrowBridgeUMGWidgetCoreCommands();

    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

    // Utility - shared with other UMG command handlers
    static FAnchors ParseAnchorPreset(const FString& AnchorStr);

private:
    TSharedPtr<FJsonObject> HandleCreateUMGWidgetBlueprint(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddWidgetToViewport(const TSharedPtr<FJsonObject>& Params);
};
