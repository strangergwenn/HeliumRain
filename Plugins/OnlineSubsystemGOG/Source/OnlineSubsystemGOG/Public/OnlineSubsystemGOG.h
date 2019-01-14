#pragma once

#include "CommonGOG.h"

#include "OnlineSubsystemImpl.h"
#include "HAL/RunnableThread.h"

class ONLINESUBSYSTEMGOG_API FOnlineSubsystemGOG : public FOnlineSubsystemImpl
{
public:

	virtual FString GetAppId() const override;

	virtual FText GetOnlineServiceName() const override;

	virtual bool Init() override;

	virtual bool IsLocalPlayer(const FUniqueNetId& InUniqueId) const override;

	virtual bool Exec(class UWorld* InWorld, const TCHAR* InCmd, FOutputDevice& OutAr) override;

	virtual bool Shutdown() override;

	virtual ~FOnlineSubsystemGOG() override;

	// Implementation of the IOnlineSubsystem

#define DECLARE_ONLINE_INTERFACE(interfaceName) \
	private: \
		IOnline##interfaceName##Ptr galaxy##interfaceName##Interface; \
		\
	public: \
		virtual IOnline##interfaceName##Ptr Get##interfaceName##Interface() const override \
		{ \
			return galaxy##interfaceName##Interface; \
		}

	DECLARE_ONLINE_INTERFACE(Identity);
	DECLARE_ONLINE_INTERFACE(Session);
	DECLARE_ONLINE_INTERFACE(Achievements);
	DECLARE_ONLINE_INTERFACE(Leaderboards);
	DECLARE_ONLINE_INTERFACE(Friends);
	DECLARE_ONLINE_INTERFACE(Presence);

#define STUB_ONLINE_INTERFACE(interfaceName) \
	virtual IOnline##interfaceName##Ptr Get##interfaceName##Interface() const override \
	{ \
		return nullptr; \
	}

	STUB_ONLINE_INTERFACE(User);

	STUB_ONLINE_INTERFACE(UserCloud);


	STUB_ONLINE_INTERFACE(Chat);

	STUB_ONLINE_INTERFACE(SharedCloud);

	STUB_ONLINE_INTERFACE(Sharing);

	STUB_ONLINE_INTERFACE(Party);

	STUB_ONLINE_INTERFACE(Groups);

	STUB_ONLINE_INTERFACE(Voice);

	STUB_ONLINE_INTERFACE(ExternalUI);

	STUB_ONLINE_INTERFACE(Time);

	STUB_ONLINE_INTERFACE(TitleFile);

	STUB_ONLINE_INTERFACE(Entitlements);

	STUB_ONLINE_INTERFACE(Store);

	STUB_ONLINE_INTERFACE(StoreV2);

	STUB_ONLINE_INTERFACE(Purchase);

	STUB_ONLINE_INTERFACE(Events);

	STUB_ONLINE_INTERFACE(Message);

	STUB_ONLINE_INTERFACE(TurnBased);

PACKAGE_SCOPE:

	FOnlineSubsystemGOG();

	FOnlineSubsystemGOG(FName InInstanceName);

	bool Tick(float InDeltaTime) override;

private:

	FString GetClientSecret() const;

	bool ReadEngineConfiguration();

	bool InitGalaxyPeer();

	void ShutdownGalaxyPeer();

	bool IsInitialized();

	bool bGalaxyPeerInitialized{false};

	FString clientID;
	FString clientSecret;

	bool ShutdownImpl();
};

using FOnlineSubsystemGOGPtr = TSharedPtr<FOnlineSubsystemGOG, ESPMode::ThreadSafe>;
