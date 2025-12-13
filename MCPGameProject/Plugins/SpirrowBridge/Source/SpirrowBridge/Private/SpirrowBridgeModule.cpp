#include "SpirrowBridgeModule.h"
#include "SpirrowBridge.h"
#include "Modules/ModuleManager.h"
#include "EditorSubsystem.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "FSpirrowBridgeModule"

void FSpirrowBridgeModule::StartupModule()
{
	UE_LOG(LogTemp, Display, TEXT("SpirrowBridge Module has started"));
}

void FSpirrowBridgeModule::ShutdownModule()
{
	UE_LOG(LogTemp, Display, TEXT("SpirrowBridge Module has shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSpirrowBridgeModule, SpirrowBridge) 