#include "Commands/SpirrowBridgeUMGWidgetCommands.h"
#include "Commands/SpirrowBridgeUMGWidgetCoreCommands.h"
#include "Commands/SpirrowBridgeUMGWidgetBasicCommands.h"
#include "Commands/SpirrowBridgeUMGWidgetInteractiveCommands.h"
#include "Commands/SpirrowBridgeCommonUtils.h"

FSpirrowBridgeUMGWidgetCommands::FSpirrowBridgeUMGWidgetCommands()
{
	CoreCommands = MakeShared<FSpirrowBridgeUMGWidgetCoreCommands>();
	BasicCommands = MakeShared<FSpirrowBridgeUMGWidgetBasicCommands>();
	InteractiveCommands = MakeShared<FSpirrowBridgeUMGWidgetInteractiveCommands>();
}

TSharedPtr<FJsonObject> FSpirrowBridgeUMGWidgetCommands::HandleCommand(const FString& CommandName, const TSharedPtr<FJsonObject>& Params)
{
	// Try Core commands first
	TSharedPtr<FJsonObject> Result = CoreCommands->HandleCommand(CommandName, Params);
	if (Result.IsValid())
	{
		return Result;
	}

	// Try Basic widget commands
	Result = BasicCommands->HandleCommand(CommandName, Params);
	if (Result.IsValid())
	{
		return Result;
	}

	// Try Interactive widget commands
	Result = InteractiveCommands->HandleCommand(CommandName, Params);
	if (Result.IsValid())
	{
		return Result;
	}

	return FSpirrowBridgeCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown UMG Widget command: %s"), *CommandName));
}

FAnchors FSpirrowBridgeUMGWidgetCommands::ParseAnchorPreset(const FString& AnchorStr)
{
	return FSpirrowBridgeUMGWidgetCoreCommands::ParseAnchorPreset(AnchorStr);
}
