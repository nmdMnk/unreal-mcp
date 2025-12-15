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
     * List GAS-related assets in the project
     */
    TSharedPtr<FJsonObject> HandleListGASAssets(const TSharedPtr<FJsonObject>& Params);

    /**
     * Create a GameplayEffect Blueprint asset
     */
    TSharedPtr<FJsonObject> HandleCreateGameplayEffect(const TSharedPtr<FJsonObject>& Params);

    /**
     * Create a GAS-enabled Character Blueprint with ASC configured
     */
    TSharedPtr<FJsonObject> HandleCreateGASCharacter(const TSharedPtr<FJsonObject>& Params);

    /**
     * Set default abilities and effects on an existing ASC
     */
    TSharedPtr<FJsonObject> HandleSetAbilitySystemDefaults(const TSharedPtr<FJsonObject>& Params);

    /**
     * Create a GameplayAbility Blueprint asset
     */
    TSharedPtr<FJsonObject> HandleCreateGameplayAbility(const TSharedPtr<FJsonObject>& Params);

    /**
     * Helper to set gameplay tag container from JSON array
     */
    void SetGameplayTagContainerFromArray(FGameplayTagContainer& Container, const TArray<TSharedPtr<FJsonValue>>* TagsArray);

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
