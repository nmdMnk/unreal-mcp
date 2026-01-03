// SpirrowBridgeConfigCommands.cpp
#include "Commands/SpirrowBridgeConfigCommands.h"
#include "Commands/SpirrowBridgeCommonUtils.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"

FSpirrowBridgeConfigCommands::FSpirrowBridgeConfigCommands()
{
}

TSharedPtr<FJsonObject> FSpirrowBridgeConfigCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("get_config_value"))
    {
        return HandleGetConfigValue(Params);
    }
    else if (CommandType == TEXT("set_config_value"))
    {
        return HandleSetConfigValue(Params);
    }
    else if (CommandType == TEXT("list_config_sections"))
    {
        return HandleListConfigSections(Params);
    }

    return FSpirrowBridgeCommonUtils::CreateErrorResponse(
        ESpirrowErrorCode::UnknownCommand,
        FString::Printf(TEXT("Unknown config command: %s"), *CommandType));
}

FString FSpirrowBridgeConfigCommands::ResolveConfigFilePath(const FString& ConfigFile, FString& OutGConfigPath, FString& OutFileName)
{
    if (ConfigFile == TEXT("DefaultEngine") || ConfigFile == TEXT("Engine"))
    {
        OutGConfigPath = GEngineIni;
        OutFileName = TEXT("DefaultEngine.ini");
    }
    else if (ConfigFile == TEXT("DefaultGame") || ConfigFile == TEXT("Game"))
    {
        OutGConfigPath = GGameIni;
        OutFileName = TEXT("DefaultGame.ini");
    }
    else if (ConfigFile == TEXT("DefaultEditor") || ConfigFile == TEXT("Editor"))
    {
        OutGConfigPath = GEditorIni;
        OutFileName = TEXT("DefaultEditor.ini");
    }
    else if (ConfigFile == TEXT("DefaultInput") || ConfigFile == TEXT("Input"))
    {
        OutGConfigPath = GInputIni;
        OutFileName = TEXT("DefaultInput.ini");
    }
    else
    {
        return FString::Printf(TEXT("Unknown config file: %s. Use DefaultEngine, DefaultGame, DefaultEditor, or DefaultInput"), *ConfigFile);
    }
    return FString();
}

TSharedPtr<FJsonObject> FSpirrowBridgeConfigCommands::HandleGetConfigValue(const TSharedPtr<FJsonObject>& Params)
{
    // Validate required parameters
    FString Section, Key;
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("section"), Section))
    {
        return Error;
    }
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("key"), Key))
    {
        return Error;
    }

    // Get optional parameters
    FString ConfigFile;
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("config_file"), ConfigFile, TEXT("DefaultEngine"));

    // Resolve config file path
    FString GConfigPath, FileName;
    FString ErrorMsg = ResolveConfigFilePath(ConfigFile, GConfigPath, FileName);
    if (!ErrorMsg.IsEmpty())
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::InvalidParameter,
            ErrorMsg);
    }

    FString Value;
    if (GConfig->GetString(*Section, *Key, Value, GConfigPath))
    {
        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetBoolField(TEXT("success"), true);
        ResultObj->SetStringField(TEXT("config_file"), ConfigFile);
        ResultObj->SetStringField(TEXT("section"), Section);
        ResultObj->SetStringField(TEXT("key"), Key);
        ResultObj->SetStringField(TEXT("value"), Value);
        return ResultObj;
    }
    else
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::ConfigKeyNotFound,
            FString::Printf(TEXT("Key not found: [%s] %s"), *Section, *Key));
    }
}

TSharedPtr<FJsonObject> FSpirrowBridgeConfigCommands::HandleSetConfigValue(const TSharedPtr<FJsonObject>& Params)
{
    // Validate required parameters
    FString Section, Key, Value;
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("section"), Section))
    {
        return Error;
    }
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("key"), Key))
    {
        return Error;
    }
    if (auto Error = FSpirrowBridgeCommonUtils::ValidateRequiredString(Params, TEXT("value"), Value))
    {
        return Error;
    }

    // Get optional parameters
    FString ConfigFile;
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("config_file"), ConfigFile, TEXT("DefaultEngine"));

    // Resolve config file path
    FString GConfigPath, FileName;
    FString ErrorMsg = ResolveConfigFilePath(ConfigFile, GConfigPath, FileName);
    if (!ErrorMsg.IsEmpty())
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::InvalidParameter,
            ErrorMsg);
    }

    FString FilePath = FPaths::ProjectConfigDir() / FileName;

    // Use FConfigFile to load, modify, and save
    FConfigFile ConfigFileObj;
    ConfigFileObj.Read(FilePath);

    ConfigFileObj.SetString(*Section, *Key, *Value);
    ConfigFileObj.Write(FilePath);

    // Also update in-memory GConfig cache
    GConfig->SetString(*Section, *Key, *Value, GConfigPath);

    UE_LOG(LogTemp, Display, TEXT("Set config value: [%s] %s = %s in %s"),
           *Section, *Key, *Value, *FilePath);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("config_file"), ConfigFile);
    ResultObj->SetStringField(TEXT("section"), Section);
    ResultObj->SetStringField(TEXT("key"), Key);
    ResultObj->SetStringField(TEXT("value"), Value);
    ResultObj->SetStringField(TEXT("file_path"), FilePath);
    return ResultObj;
}

TSharedPtr<FJsonObject> FSpirrowBridgeConfigCommands::HandleListConfigSections(const TSharedPtr<FJsonObject>& Params)
{
    // Get optional parameters
    FString ConfigFile;
    FSpirrowBridgeCommonUtils::GetOptionalString(Params, TEXT("config_file"), ConfigFile, TEXT("DefaultEngine"));

    // Resolve config file path
    FString GConfigPath, FileName;
    FString ErrorMsg = ResolveConfigFilePath(ConfigFile, GConfigPath, FileName);
    if (!ErrorMsg.IsEmpty())
    {
        return FSpirrowBridgeCommonUtils::CreateErrorResponse(
            ESpirrowErrorCode::InvalidParameter,
            ErrorMsg);
    }

    TArray<FString> SectionNames;
    GConfig->GetSectionNames(GConfigPath, SectionNames);

    TArray<TSharedPtr<FJsonValue>> SectionsArray;
    for (const FString& SectionName : SectionNames)
    {
        SectionsArray.Add(MakeShared<FJsonValueString>(SectionName));
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("config_file"), ConfigFile);
    ResultObj->SetArrayField(TEXT("sections"), SectionsArray);
    ResultObj->SetNumberField(TEXT("count"), SectionNames.Num());
    return ResultObj;
}
