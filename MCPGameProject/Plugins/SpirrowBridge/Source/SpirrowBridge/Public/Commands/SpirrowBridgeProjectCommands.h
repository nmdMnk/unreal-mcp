#pragma once

#include "CoreMinimal.h"
#include "Json.h"

/**
 * Handler class for Project-wide MCP commands
 */
class SPIRROWBRIDGE_API FSpirrowBridgeProjectCommands
{
public:
    FSpirrowBridgeProjectCommands();

    // Handle project commands
    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
    // Specific project command handlers
    TSharedPtr<FJsonObject> HandleCreateInputMapping(const TSharedPtr<FJsonObject>& Params);

    // Enhanced Input System handlers
    TSharedPtr<FJsonObject> HandleCreateInputAction(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleCreateInputMappingContext(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddActionToMappingContext(const TSharedPtr<FJsonObject>& Params);
}; 