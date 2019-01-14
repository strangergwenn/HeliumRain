#pragma once

#include "OnlineFactoryGOG.h"

#include "CoreMinimal.h"
#include "ModuleManager.h"
#include "Modules/ModuleInterface.h"

#include "UniquePtr.h"

// Online subsystem module for GOG online services.

// This class is an entry point from UE4. See base class for details.
class FOnlineSubsystemGOGModule : public IModuleInterface
{
public:

	FOnlineSubsystemGOGModule();

	virtual ~FOnlineSubsystemGOGModule();

	// Entry point for this class. The exact time this method is executed depends on configuration from .uplugin
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

private:

	// Register OnlineSubsystemGOG with the base level factory provider for easy access
	void RegisterOnlineSubsystem();

	void UnRegisterOnlineSubsystem();

	// Handle for the factory
	TUniquePtr<FOnlineFactoryGOG> onlineFactoryGOG;

	// Handle for the GalaxySDK dll
	void* galaxySdkDllHandle;
};