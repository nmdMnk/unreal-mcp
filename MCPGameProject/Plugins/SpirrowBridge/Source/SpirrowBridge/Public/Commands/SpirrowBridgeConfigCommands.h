#pragma once

#include "CoreMinimal.h"
#include "Json.h"

/**
 * Handler class for Config file (ini) related MCP commands
 * Handles reading and writing project configuration files
 */
class SPIRROWBRIDGE_API FSpirrowBridgeConfigCommands
{
public:
    FSpirrowBridgeConfigCommands();

    // Handle config commands
    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
    // Config file commands
    TSharedPtr<FJsonObject> HandleGetConfigValue(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleSetConfigValue(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleListConfigSections(const TSharedPtr<FJsonObject>& Params);

    // Helper to resolve config file path
    FString ResolveConfigFilePath(const FString& ConfigFile, FString& OutGConfigPath, FString& OutFileName);
};
