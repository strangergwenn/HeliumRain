#pragma once

#include "CommonGOG.h"
#include "NetConnectionGOG.h"

#include "Engine/NetDriver.h"
#include <array>

#include "NetDriverGOG.generated.h"

constexpr uint32_t PREALLOCATED_BUFFER_SIZE = 4096;

UCLASS(transient, config = Engine)
class UNetDriverGOG
	: public UNetDriver
{

	GENERATED_BODY()

public:

	bool IsAvailable() const override;

	bool IsNetResourceValid() override;

	bool InitBase(bool InInitAsClient, FNetworkNotify* InNotify, const FURL& InURL, bool InReuseAddressAndPort, FString& OutError) override;

	bool InitConnect(FNetworkNotify* InNotify, const FURL& InConnectURL, FString& OutError) override;

	bool InitListen(FNetworkNotify* InNotify, FURL& InLocalURL, bool InReuseAddressAndPort, FString& OutError) override;

	void ProcessRemoteFunction(class AActor* InActor, class UFunction* InFunction, void* InParameters, struct FOutParmRec* OutParms, struct FFrame* InStack, class UObject* InSubObject = NULL) override;

	void TickDispatch(float InDeltaTime) override;

	void LowLevelSend(FString InAddress, void* InData, int32 InCountBits) override;

	FString LowLevelGetNetworkNumber() override;

	class ISocketSubsystem* GetSocketSubsystem() override;

private:

	bool ChallengeConnectingClient(const class FUrlGOG& InRemoteUrl, void* InData, uint32_t InDataSize);

	UNetConnectionGOG* EstablishIncomingConnection(const class FUrlGOG& InRemoteUrl);

	UNetConnection* FindEstablishedConnection(const FUrlGOG& InRemoteUrl);

	class LobbyLeftListener
		: public galaxy::api::GlobalLobbyLeftListener
	{
	public:

		LobbyLeftListener(UNetDriverGOG& InDriver);

		void OnLobbyLeft(const galaxy::api::GalaxyID& InLobbyID, bool InIOFailure) override;

		UNetDriverGOG& driver;
	};

	TUniquePtr<LobbyLeftListener> lobbyLeftListener;

	galaxy::api::INetworking* galaxyNetworking{nullptr};

	std::array<uint8, PREALLOCATED_BUFFER_SIZE> inPacketBuffer;
};
