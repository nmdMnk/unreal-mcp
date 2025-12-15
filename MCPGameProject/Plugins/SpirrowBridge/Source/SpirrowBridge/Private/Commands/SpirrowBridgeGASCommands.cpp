#include "Commands/SpirrowBridgeGASCommands.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/JsonSerializer.h"

FSpirrowBridgeGASCommands::FSpirrowBridgeGASCommands()
{
}

FSpirrowBridgeGASCommands::~FSpirrowBridgeGASCommands()
{
}

TSharedPtr<FJsonObject> FSpirrowBridgeGASCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("add_gameplay_tags"))
    {
        return HandleAddGameplayTags(Params);
    }
    else if (CommandType == TEXT("list_gameplay_tags"))
    {
        return HandleListGameplayTags(Params);
    }
    else if (CommandType == TEXT("remove_gameplay_tag"))
    {
        return HandleRemoveGameplayTag(Params);
    }

    TSharedPtr<FJsonObject> ErrorResponse = MakeShareable(new FJsonObject);
    ErrorResponse->SetBoolField(TEXT("success"), false);
    ErrorResponse->SetStringField(TEXT("error"), FString::Printf(TEXT("Unknown GAS command: %s"), *CommandType));
    return ErrorResponse;
}

FString FSpirrowBridgeGASCommands::GetGameplayTagsConfigPath() const
{
    return FPaths::ProjectConfigDir() / TEXT("DefaultGameplayTags.ini");
}

TArray<TPair<FString, FString>> FSpirrowBridgeGASCommands::ParseExistingTags(const FString& ConfigPath)
{
    TArray<TPair<FString, FString>> ExistingTags;

    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *ConfigPath))
    {
        return ExistingTags;
    }

    // Parse lines looking for GameplayTagList entries
    // Format: +GameplayTagList=(Tag="TagName",DevComment="Comment")
    TArray<FString> Lines;
    FileContent.ParseIntoArrayLines(Lines);

    for (const FString& Line : Lines)
    {
        if (Line.Contains(TEXT("GameplayTagList=")))
        {
            // Extract Tag value
            FString TagValue;
            FString CommentValue;

            // Find Tag="..."
            int32 TagStart = Line.Find(TEXT("Tag=\""));
            if (TagStart != INDEX_NONE)
            {
                TagStart += 5; // Length of 'Tag="'
                int32 TagEnd = Line.Find(TEXT("\""), ESearchCase::IgnoreCase, ESearchDir::FromStart, TagStart);
                if (TagEnd != INDEX_NONE)
                {
                    TagValue = Line.Mid(TagStart, TagEnd - TagStart);
                }
            }

            // Find DevComment="..."
            int32 CommentStart = Line.Find(TEXT("DevComment=\""));
            if (CommentStart != INDEX_NONE)
            {
                CommentStart += 12; // Length of 'DevComment="'
                int32 CommentEnd = Line.Find(TEXT("\""), ESearchCase::IgnoreCase, ESearchDir::FromStart, CommentStart);
                if (CommentEnd != INDEX_NONE)
                {
                    CommentValue = Line.Mid(CommentStart, CommentEnd - CommentStart);
                }
            }

            if (!TagValue.IsEmpty())
            {
                ExistingTags.Add(TPair<FString, FString>(TagValue, CommentValue));
            }
        }
    }

    return ExistingTags;
}

bool FSpirrowBridgeGASCommands::WriteTagsToConfig(const FString& ConfigPath, const TArray<TPair<FString, FString>>& Tags)
{
    FString FileContent;
    FFileHelper::LoadFileToString(FileContent, *ConfigPath);

    // Find or create the GameplayTags section
    const FString SectionName = TEXT("[/Script/GameplayTags.GameplayTagsSettings]");

    // Remove existing GameplayTagList entries
    TArray<FString> Lines;
    FileContent.ParseIntoArrayLines(Lines);

    TArray<FString> NewLines;
    bool bInSection = false;
    bool bSectionFound = false;

    for (const FString& Line : Lines)
    {
        if (Line.StartsWith(TEXT("[")))
        {
            bInSection = Line.Equals(SectionName);
            if (bInSection)
            {
                bSectionFound = true;
            }
        }

        // Skip existing GameplayTagList entries in our section
        if (bInSection && Line.Contains(TEXT("GameplayTagList=")))
        {
            continue;
        }

        NewLines.Add(Line);
    }

    // If section doesn't exist, add it
    if (!bSectionFound)
    {
        NewLines.Add(TEXT(""));
        NewLines.Add(SectionName);
    }

    // Find where to insert new tags (after section header)
    int32 InsertIndex = NewLines.IndexOfByPredicate([&SectionName](const FString& Line) {
        return Line.Equals(SectionName);
    });

    if (InsertIndex != INDEX_NONE)
    {
        InsertIndex++; // Insert after section header

        // Add all tags
        for (const auto& TagPair : Tags)
        {
            FString TagLine;
            if (TagPair.Value.IsEmpty())
            {
                TagLine = FString::Printf(TEXT("+GameplayTagList=(Tag=\"%s\")"), *TagPair.Key);
            }
            else
            {
                TagLine = FString::Printf(TEXT("+GameplayTagList=(Tag=\"%s\",DevComment=\"%s\")"), *TagPair.Key, *TagPair.Value);
            }
            NewLines.Insert(TagLine, InsertIndex++);
        }
    }

    // Write back to file
    FString NewContent = FString::Join(NewLines, TEXT("\n"));
    return FFileHelper::SaveStringToFile(NewContent, *ConfigPath);
}

TSharedPtr<FJsonObject> FSpirrowBridgeGASCommands::HandleAddGameplayTags(const TSharedPtr<FJsonObject>& Params)
{
    TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject);

    // Get tags array from params
    const TArray<TSharedPtr<FJsonValue>>* TagsArray;
    if (!Params->TryGetArrayField(TEXT("tags"), TagsArray))
    {
        Response->SetBoolField(TEXT("success"), false);
        Response->SetStringField(TEXT("error"), TEXT("Missing 'tags' parameter"));
        return Response;
    }

    // Get optional comments
    const TSharedPtr<FJsonObject>* CommentsObj = nullptr;
    Params->TryGetObjectField(TEXT("comments"), CommentsObj);

    FString ConfigPath = GetGameplayTagsConfigPath();

    // Parse existing tags
    TArray<TPair<FString, FString>> ExistingTags = ParseExistingTags(ConfigPath);
    TSet<FString> ExistingTagSet;
    for (const auto& TagPair : ExistingTags)
    {
        ExistingTagSet.Add(TagPair.Key);
    }

    // Process new tags
    TArray<FString> AddedTags;
    TArray<FString> SkippedTags;

    for (const TSharedPtr<FJsonValue>& TagValue : *TagsArray)
    {
        FString Tag = TagValue->AsString();

        if (ExistingTagSet.Contains(Tag))
        {
            SkippedTags.Add(Tag);
            continue;
        }

        // Get comment if provided
        FString Comment;
        if (CommentsObj && (*CommentsObj)->HasField(Tag))
        {
            Comment = (*CommentsObj)->GetStringField(Tag);
        }

        ExistingTags.Add(TPair<FString, FString>(Tag, Comment));
        ExistingTagSet.Add(Tag);
        AddedTags.Add(Tag);
    }

    // Write back to config
    if (AddedTags.Num() > 0)
    {
        if (!WriteTagsToConfig(ConfigPath, ExistingTags))
        {
            Response->SetBoolField(TEXT("success"), false);
            Response->SetStringField(TEXT("error"), TEXT("Failed to write config file"));
            return Response;
        }
    }

    // Build response
    Response->SetBoolField(TEXT("success"), true);
    Response->SetStringField(TEXT("file_path"), ConfigPath);

    TArray<TSharedPtr<FJsonValue>> AddedArray;
    for (const FString& Tag : AddedTags)
    {
        AddedArray.Add(MakeShareable(new FJsonValueString(Tag)));
    }
    Response->SetArrayField(TEXT("added_tags"), AddedArray);

    TArray<TSharedPtr<FJsonValue>> SkippedArray;
    for (const FString& Tag : SkippedTags)
    {
        SkippedArray.Add(MakeShareable(new FJsonValueString(Tag)));
    }
    Response->SetArrayField(TEXT("skipped_tags"), SkippedArray);

    UE_LOG(LogTemp, Display, TEXT("SpirrowBridge: Added %d gameplay tags, skipped %d existing"), AddedTags.Num(), SkippedTags.Num());

    return Response;
}

TSharedPtr<FJsonObject> FSpirrowBridgeGASCommands::HandleListGameplayTags(const TSharedPtr<FJsonObject>& Params)
{
    TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject);

    FString FilterPrefix = Params->GetStringField(TEXT("filter_prefix"));
    FString ConfigPath = GetGameplayTagsConfigPath();

    TArray<TPair<FString, FString>> AllTags = ParseExistingTags(ConfigPath);

    TArray<TSharedPtr<FJsonValue>> TagsArray;
    for (const auto& TagPair : AllTags)
    {
        // Apply filter if specified
        if (!FilterPrefix.IsEmpty() && !TagPair.Key.StartsWith(FilterPrefix))
        {
            continue;
        }

        TSharedPtr<FJsonObject> TagObj = MakeShareable(new FJsonObject);
        TagObj->SetStringField(TEXT("tag"), TagPair.Key);
        TagObj->SetStringField(TEXT("comment"), TagPair.Value);
        TagsArray.Add(MakeShareable(new FJsonValueObject(TagObj)));
    }

    Response->SetBoolField(TEXT("success"), true);
    Response->SetArrayField(TEXT("tags"), TagsArray);
    Response->SetNumberField(TEXT("count"), TagsArray.Num());
    Response->SetStringField(TEXT("file_path"), ConfigPath);

    return Response;
}

TSharedPtr<FJsonObject> FSpirrowBridgeGASCommands::HandleRemoveGameplayTag(const TSharedPtr<FJsonObject>& Params)
{
    TSharedPtr<FJsonObject> Response = MakeShareable(new FJsonObject);

    FString TagToRemove = Params->GetStringField(TEXT("tag"));
    if (TagToRemove.IsEmpty())
    {
        Response->SetBoolField(TEXT("success"), false);
        Response->SetStringField(TEXT("error"), TEXT("Missing 'tag' parameter"));
        return Response;
    }

    FString ConfigPath = GetGameplayTagsConfigPath();
    TArray<TPair<FString, FString>> ExistingTags = ParseExistingTags(ConfigPath);

    bool bFound = false;
    TArray<TPair<FString, FString>> NewTags;
    for (const auto& TagPair : ExistingTags)
    {
        if (TagPair.Key.Equals(TagToRemove))
        {
            bFound = true;
            continue;
        }
        NewTags.Add(TagPair);
    }

    if (bFound)
    {
        if (!WriteTagsToConfig(ConfigPath, NewTags))
        {
            Response->SetBoolField(TEXT("success"), false);
            Response->SetStringField(TEXT("error"), TEXT("Failed to write config file"));
            return Response;
        }
    }

    Response->SetBoolField(TEXT("success"), true);
    Response->SetBoolField(TEXT("removed"), bFound);
    Response->SetStringField(TEXT("message"), bFound ? FString::Printf(TEXT("Removed tag: %s"), *TagToRemove) : FString::Printf(TEXT("Tag not found: %s"), *TagToRemove));
    Response->SetStringField(TEXT("file_path"), ConfigPath);

    return Response;
}
