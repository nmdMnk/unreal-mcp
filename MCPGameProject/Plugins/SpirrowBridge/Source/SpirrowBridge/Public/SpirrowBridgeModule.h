#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FSpirrowBridgeModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static inline FSpirrowBridgeModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FSpirrowBridgeModule>("SpirrowBridge");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("SpirrowBridge");
	}
}; 