#include "OnlineFactoryGOG.h"

FOnlineFactoryGOG::FOnlineFactoryGOG()
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineFactoryGOG::ctor()"));
}

IOnlineSubsystemPtr FOnlineFactoryGOG::CreateSubsystem(FName InInstanceName)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineFactoryGOG::CreateSubsystem(%s)"), *InInstanceName.ToString());

	if (onlineSubsystemGOG.IsValid())
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSubsystemGOG was already created"));
		return onlineSubsystemGOG;
	}

	onlineSubsystemGOG = MakeShared<FOnlineSubsystemGOG, ESPMode::ThreadSafe>(InInstanceName);
	if (!onlineSubsystemGOG->Init())
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to initialize OnlineSubsystemGOG!"));
		DestroyOnlineSubsystemGOG();
	}

	return onlineSubsystemGOG;
}

void FOnlineFactoryGOG::DestroyOnlineSubsystemGOG()
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineFactoryGOG::DestroyOnlineSubsystemGOG()"));

	if (!onlineSubsystemGOG.IsValid())
	{
		UE_LOG_ONLINE(Display, TEXT("FOnlineSubsystemGOG already expired"));
		return;
	}

	onlineSubsystemGOG->Shutdown();
	onlineSubsystemGOG.Reset();
}

FOnlineFactoryGOG::~FOnlineFactoryGOG()
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineFactoryGOG::dtor()"));

	DestroyOnlineSubsystemGOG();
}
