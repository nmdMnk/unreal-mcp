#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

/**
 * Handles GAS (Gameplay Ability System) related commands for SpirrowBridge.
 */
class SPIRROWBRIDGE_API FSpirrowBridgeGASCommands
{
public:
    FSpirrowBridgeGASCommands();
    ~FSpirrowBridgeGASCommands();

    /**
     * Main command handler that routes to specific handlers.
     */
    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
    /**
     * Add gameplay tags to DefaultGameplayTags.ini
     */
    TSharedPtr<FJsonObject> HandleAddGameplayTags(const TSharedPtr<FJsonObject>& Params);

    /**
     * List all gameplay tags from DefaultGameplayTags.ini
     */
    TSharedPtr<FJsonObject> HandleListGameplayTags(const TSharedPtr<FJsonObject>& Params);

    /**
     * Remove a gameplay tag from DefaultGameplayTags.ini
     */
    TSharedPtr<FJsonObject> HandleRemoveGameplayTag(const TSharedPtr<FJsonObject>& Params);

    /**
     * Get the path to DefaultGameplayTags.ini
     */
    FString GetGameplayTagsConfigPath() const;

    /**
     * Parse existing tags from the config file
     */
    TArray<TPair<FString, FString>> ParseExistingTags(const FString& ConfigPath);

    /**
     * Write tags back to the config file
     */
    bool WriteTagsToConfig(const FString& ConfigPath, const TArray<TPair<FString, FString>>& Tags);
};
