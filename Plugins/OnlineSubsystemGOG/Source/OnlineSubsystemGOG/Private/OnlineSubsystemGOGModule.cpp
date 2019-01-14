#include "OnlineSubsystemGOGModule.h"

#include "CommonGOG.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

namespace
{

	const FString GetGalaxySdkLibraryPath()
	{
		const auto pluginRootDir = IPluginManager::Get().FindPlugin(TEXT_ONLINE_SUBSYSTEM_GOG)->GetBaseDir();

		if (pluginRootDir.IsEmpty())
			UE_LOG_ONLINE(Error, TEXT("Cannot find base plugin directory for %s"), TEXT_ONLINE_SUBSYSTEM_GOG);

		auto galaxySdkDllPath = FPaths::Combine(
			pluginRootDir,
			TEXT("Source"),
			TEXT("ThirdParty"),
			TEXT("GalaxySDK"),
			TEXT("Libraries"),
			TEXT(STRINGIFY(GALAXY_DLL_NAME)));

		if (!FPaths::FileExists(galaxySdkDllPath))
			UE_LOG_ONLINE(Error, TEXT("Cannot find GalaxySDK dll (%s)"), *galaxySdkDllPath);

		return galaxySdkDllPath;
	}
}

// Inform UE4 that this is the implementation of the OnlineSubsystemGOG module
IMPLEMENT_MODULE(FOnlineSubsystemGOGModule, OnlineSubsystemGOG)

FOnlineSubsystemGOGModule::FOnlineSubsystemGOGModule()
	: galaxySdkDllHandle{nullptr}
{
	UE_LOG_ONLINE(Verbose, TEXT("OnlineSubsystemGOGModule::ctor()"));
}

void FOnlineSubsystemGOGModule::RegisterOnlineSubsystem()
{
	UE_LOG_ONLINE(Verbose, TEXT("OnlineSubsystemGOGModule::RegisterOnlineSubsystem()"));

	check(!onlineFactoryGOG.IsValid() && "FOnlineFactoryGOG already created (and probably registered)");

	onlineFactoryGOG.Reset(new FOnlineFactoryGOG());

	check(onlineFactoryGOG.IsValid() && "Failed to create FOnlineFactoryGOG factory");

	FModuleManager::GetModuleChecked<FOnlineSubsystemModule>(TEXT("OnlineSubsystem"))
		.RegisterPlatformService(TEXT_GOG, onlineFactoryGOG.Get());
}

void FOnlineSubsystemGOGModule::StartupModule()
{
	UE_LOG_ONLINE(Verbose, TEXT("OnlineSubsystemGOGModule::StartupModule()"));

	check(!galaxySdkDllHandle && "GalaxySDK already loaded");

	galaxySdkDllHandle = FPlatformProcess::GetDllHandle(*GetGalaxySdkLibraryPath());
	if (!galaxySdkDllHandle)
	{
		UE_LOG_ONLINE(Error, TEXT("Failed to load GalaxySDK library: libraryPath=%s"), *GetGalaxySdkLibraryPath());
		return;
	}

	RegisterOnlineSubsystem();
}

void FOnlineSubsystemGOGModule::UnRegisterOnlineSubsystem()
{
	UE_LOG_ONLINE(Verbose, TEXT("OnlineSubsystemGOGModule::UnRegisterOnlineSubsystem()"));

	if (!onlineFactoryGOG.IsValid())
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineFactoryGOG was already deleted"));
		return;
	}

	FModuleManager::GetModuleChecked<FOnlineSubsystemModule>(TEXT("OnlineSubsystem"))
		.UnregisterPlatformService(TEXT_GOG);

	onlineFactoryGOG.Reset();
}

void FOnlineSubsystemGOGModule::ShutdownModule()
{
	UE_LOG_ONLINE(Verbose, TEXT("OnlineSubsystemGOGModule::ShutdownModule()"));

	UnRegisterOnlineSubsystem();

	FPlatformProcess::FreeDllHandle(galaxySdkDllHandle);
	galaxySdkDllHandle = nullptr;
}

FOnlineSubsystemGOGModule::~FOnlineSubsystemGOGModule()
{
	UE_LOG_ONLINE(Verbose, TEXT("OnlineSubsystemGOGModule::dtor()"));

	check(!onlineFactoryGOG.IsValid() && "FOnlineFactoryGOG was not deleted (and probably still registered)");
	check(!galaxySdkDllHandle && "GalaxySDK library was not released");
}
