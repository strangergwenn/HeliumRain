#include "NetConnectionGOG.h"

#include "CommonGOG.h"
#include "Types/UrlGOG.h"
#include "Loggers.h"

#include "Net/DataChannel.h"
#include "StatelessConnectHandlerComponent.h"

void UNetConnectionGOG::InitBase(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
	UE_LOG_NETWORKING(Log, TEXT("UNetConnectionGOG::InitBase()"));

	UNetConnection::InitBase(InDriver, InSocket, InURL, InState, InMaxPacket == 0 ? MAX_PACKET_SIZE : InMaxPacket, 1);

	// We handle our own overhead

	PacketOverhead = 0;

	InitSendBuffer();
}

void UNetConnectionGOG::InitLocalConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
	UE_LOG_NETWORKING(Log, TEXT("UNetConnectionGOG::InitLocalConnection()"));

	remotePeerID = FUniqueNetIdGOG{InURL.Host};

	checkf(remotePeerID.IsValid() && remotePeerID.IsLobby(), TEXT("Remote PeerID expected to be a lobby: remotePeerID='%s', remoteUrl='%s'"),
		*remotePeerID.ToString(), *InURL.ToString(true));

	galaxyNetworking = galaxy::api::Networking();
	check(galaxyNetworking);

	InitBase(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);
}

void UNetConnectionGOG::InitRemoteConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, const class FInternetAddr& InRemoteAddr, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
	UE_LOG_NETWORKING(Log, TEXT("UNetConnectionGOG::InitRemoteConnection()"));

	remotePeerID = FUniqueNetIdGOG{InURL.Host};
	checkf(remotePeerID.IsValid() && remotePeerID.IsUser(), TEXT("Remote PeerID expected to be an user: remotePeerID='%s', remoteUrl='%s'"),
		*remotePeerID.ToString(), *InURL.ToString(true));

	galaxyNetworking = galaxy::api::ServerNetworking();
	check(galaxyNetworking);

	InitBase(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);

	serverMemberStateListener.Reset(new LobbyMemberStateListener{*this});

	SetClientLoginState(EClientLoginState::LoggingIn);
	SetExpectedClientLoginMsgType(NMT_Hello);
}

void UNetConnectionGOG::LowLevelSend(void* InData, int32 /*InCountBits*/, int32 InCountBits)
{
	UE_LOG_TRAFFIC(VeryVerbose, TEXT("UNetConnectionGOG::LowLevelSend()"));

	if (!remotePeerID.IsValid())
	{
		UE_LOG_TRAFFIC(Error, TEXT("Invalid remote PeerID: peerID=%s, URL=%s"), *remotePeerID.ToString(), *URL.ToString(true));
		return;
	}

	auto dataToSend = reinterpret_cast<uint8*>(InData);

	if (Handler.IsValid() && !Handler->GetRawSend())
	{
		const auto processedDataPacket = Handler->Outgoing(dataToSend, InCountBits);

		if (processedDataPacket.bError)
		{
			UE_LOG_TRAFFIC(Error, TEXT("Error processing packet with connectionless handler"));
			return;
		}

		dataToSend = processedDataPacket.Data;
		InCountBits = processedDataPacket.CountBits;
	}

	auto bytesToSend = FMath::DivideAndRoundUp(InCountBits, 8);

#if !UE_BUILD_SHIPPING
	bool bBlockSend = false;
	LowLevelSendDel.ExecuteIfBound(InData, bytesToSend, bBlockSend);
	if (bBlockSend)
	{
		UE_LOG_TRAFFIC(Log, TEXT("Packet sending is blocked"));
		return;
	}
#endif

	if (bytesToSend <= 0)
	{
		UE_LOG_TRAFFIC(Log, TEXT("No data to send. Skipping"));
		return;
	}

	UE_LOG_TRAFFIC(VeryVerbose, TEXT("Low level send: remote='%s'; dataSize='%d' bytes"), *LowLevelGetRemoteAddress(), bytesToSend);

	galaxyNetworking->SendP2PPacket(remotePeerID, dataToSend, bytesToSend, galaxy::api::P2P_SEND_UNRELIABLE_IMMEDIATE);
	auto err = galaxy::api::GetError();
	if (err)
		UE_LOG_TRAFFIC(Error, TEXT("Failed to send data: remote='%s'; %s; %s"), *LowLevelGetRemoteAddress(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
}

FString UNetConnectionGOG::LowLevelGetRemoteAddress(bool InAppendPort)
{
	UE_LOG_TRAFFIC(VeryVerbose, TEXT("UNetConnectionGOG::LowLevelGetRemoteAddress()"));

	return URL.ToString(InAppendPort);
}

FString UNetConnectionGOG::LowLevelDescribe()
{
	UE_LOG_TRAFFIC(VeryVerbose, TEXT("UNetConnectionGOG::LowLevelDescribe()"));

	return LowLevelGetRemoteAddress();
}

FString UNetConnectionGOG::RemoteAddressToString()
{
	UE_LOG_TRAFFIC(VeryVerbose, TEXT("UNetConnectionGOG::LowLevelSend()"));

	return LowLevelGetRemoteAddress(/* bAppendPort */ true);
}

UNetConnectionGOG::LobbyMemberStateListener::LobbyMemberStateListener(UNetConnectionGOG& InConnection)
	: connection{InConnection}
{
	check(connection.Driver != nullptr);
	check(connection.Driver->IsServer() && "LobbyMemberStateChanged should only be handled by the Server");
}

void UNetConnectionGOG::LobbyMemberStateListener::OnLobbyMemberStateChanged(const galaxy::api::GalaxyID& InLobbyID, const galaxy::api::GalaxyID& InMemberID, galaxy::api::LobbyMemberStateChange InMemberStateChange)
{
	UE_LOG_NETWORKING(Log, TEXT("UNetConnectionGOG::OnLobbyMemberStateChanged()"));

	if (InMemberID != connection.remotePeerID)
	{
		UE_LOG_NETWORKING(Verbose, TEXT("Lobby member from another connection. Ignoring"));
		return;
	}

	if (InMemberStateChange == galaxy::api::LOBBY_MEMBER_STATE_CHANGED_LEFT
		|| InMemberStateChange == galaxy::api::LOBBY_MEMBER_STATE_CHANGED_DISCONNECTED
		|| InMemberStateChange == galaxy::api::LOBBY_MEMBER_STATE_CHANGED_KICKED)
	{
		// Unregister listener as connection object destruction is delayed until garbage collection
		galaxy::api::ListenerRegistrar()->Unregister(GetListenerType(), this);

		// Closing the connection will eventually destroy a session tied player controller is in
		connection.Close();
	}
}
