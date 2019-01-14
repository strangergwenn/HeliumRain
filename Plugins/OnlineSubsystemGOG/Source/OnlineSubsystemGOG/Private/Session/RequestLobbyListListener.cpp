#include "RequestLobbyListListener.h"

#include "OnlineSessionGOG.h"
#include "VariantDataUtils.h"
#include "OnlineSessionUtils.h"

#include "Online.h"

namespace
{

	bool IsFilterEnabled(const FSearchParams& InSearchQueryParams, const FName& InParamName)
	{
		const auto* searchParam = InSearchQueryParams.Find(InParamName);

		if (!searchParam
			|| searchParam->Data.GetType() != EOnlineKeyValuePairDataType::Bool)
			return false;

		bool isEnabled;
		searchParam->Data.GetValue(isEnabled);
		return isEnabled;
	}

	bool MinSlotsAvailable(const FOnlineSessionSearchParam* const InSearchParam, int32 InOpenPublicConnections)
	{
		int32 minSlotsRequired;
		if (!SafeGetInt32Value(InSearchParam->Data, minSlotsRequired))
		{
			UE_LOG_ONLINE(Warning, TEXT("Invalid data type. Skipping SEARCH_MINSLOTSAVAILABLE filter: %s"), *InSearchParam->ToString());
			return true;
		};

		return minSlotsRequired >= InOpenPublicConnections;
	}

	bool IsUserInLobby(const FOnlineSessionSearchParam* const InSearchParam, int32 InTotalUserCount, const FOnlineSessionSearchResult &InSearchResult)
	{
		FUniqueNetIdGOG userID{InSearchParam->Data.ToString()};
		galaxy::api::GalaxyID galaxyUserID{userID};

		if (!galaxyUserID.IsValid() || galaxyUserID.GetIDType() != galaxy::api::GalaxyID::ID_TYPE_USER)
		{
			UE_LOG_ONLINE(Warning, TEXT("Invalid UserID. Skipping SEARCH_USER filter: userID=%llu"), *userID.ToString());
			return true;
		}

		for (int32 playerID{0}; playerID < InTotalUserCount; ++playerID)
		{
			const auto lobbyMemberID = galaxy::api::Matchmaking()->GetLobbyMemberByIndex(FUniqueNetIdGOG{InSearchResult.Session.SessionInfo->GetSessionId()}, playerID);
			auto err = galaxy::api::GetError();
			if (err)
			{
				UE_LOG_ONLINE(Warning, TEXT("Failed to get lobby members for lobby. Skipping SEARCH_USER filter: sessionID=%s; %s; %s"),
					*InSearchResult.Session.GetSessionIdStr(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
				return true;
			}

			if (galaxyUserID == lobbyMemberID)
				return true;
		}

		return false;
	}

	bool TestFilters(const FSearchParams& InSearchQueryParams, const FOnlineSessionSearchResult& InSearchResult)
	{
		int32 maxPublicConnections = InSearchResult.Session.SessionSettings.NumPublicConnections;
		int32 openPublicConnections = InSearchResult.Session.NumOpenPublicConnections;

		if (IsFilterEnabled(InSearchQueryParams, SEARCH_EMPTY_SERVERS_ONLY) && openPublicConnections != maxPublicConnections)
			return false;

		if (IsFilterEnabled(InSearchQueryParams, SEARCH_NONEMPTY_SERVERS_ONLY) && openPublicConnections == maxPublicConnections)
			return false;

		const auto* searchParam = InSearchQueryParams.Find(SEARCH_MINSLOTSAVAILABLE);
		if (searchParam && !MinSlotsAvailable(searchParam, openPublicConnections))
			return false;

		searchParam = InSearchQueryParams.Find(SEARCH_USER);
		if (searchParam && !IsUserInLobby(searchParam, maxPublicConnections - openPublicConnections, InSearchResult))
			return false;

		return true;
	}

	FString ToString(galaxy::api::LobbyListResult lobbyListResult)
	{
		switch (lobbyListResult)
		{
			case galaxy::api::LobbyListResult::LOBBY_LIST_RESULT_SUCCESS:
				return TEXT("The list of lobbies retrieved successfully.");
			case galaxy::api::LobbyListResult::LOBBY_LIST_RESULT_CONNECTION_FAILURE:
				return TEXT("Unable to communicate with backend services.");
			case galaxy::api::LobbyListResult::LOBBY_LIST_RESULT_ERROR:
			default:
				return TEXT("Unexpected error.");
		}
	}

}

FRequestLobbyListListener::FRequestLobbyListListener(
	class FOnlineSessionGOG& InSessionInterface,
	TSharedRef<FOnlineSessionSearch> InOutSearchSettings,
	FSearchParams InPostOperationSearchQueryParams)
	: sessionInterface{InSessionInterface}
	, searchSettings{MoveTemp(InOutSearchSettings)}
	, postOperationSearchQueryParams{InPostOperationSearchQueryParams}
{
}

void FRequestLobbyListListener::TriggerOnFindSessionsCompleteDelegates(bool InIsSuccessful)
{
	searchSettings->SearchState = InIsSuccessful
		? EOnlineAsyncTaskState::Done
		: EOnlineAsyncTaskState::Failed;

	sessionInterface.TriggerOnFindSessionsCompleteDelegates(InIsSuccessful);

	sessionInterface.FreeListener(MoveTemp(ListenerID));
}

void FRequestLobbyListListener::OnLobbyList(uint32_t InLobbyCount, galaxy::api::LobbyListResult lobbyListResult)
{
	UE_LOG_ONLINE(Display, TEXT("FRequestLobbyListListener::OnLobbyList()"));

	if (lobbyListResult != galaxy::api::LOBBY_LIST_RESULT_SUCCESS)
	{
		UE_LOG_ONLINE(Error, TEXT("Failed to retrieve lobby list: %s"), *ToString(lobbyListResult));

		TriggerOnFindSessionsCompleteDelegates(false);
		return;
	}

	if (InLobbyCount == 0)
	{
		UE_LOG_ONLINE(Display, TEXT("Empty lobby list"));
		searchSettings->SearchResults.Empty();
		TriggerOnFindSessionsCompleteDelegates(true);
		return;
	}

	if (!RequestLobbiesData(InLobbyCount))
	{
		TriggerOnFindSessionsCompleteDelegates(false);
		return;
	};

	UE_LOG_ONLINE(Display, TEXT("Waiting for lobby data to be retrieved: pendingLobbyListSize=%d"), pendingLobbyList.Num());
}

bool FRequestLobbyListListener::RequestLobbiesData(uint32_t InLobbyCount)
{
	for (uint32_t lobbbyIdx = 0; lobbbyIdx < InLobbyCount; ++lobbbyIdx)
	{
		auto lobbyID = galaxy::api::Matchmaking()->GetLobbyByIndex(lobbbyIdx);
		auto err = galaxy::api::GetError();
		if (err)
		{
			UE_LOG_ONLINE(Warning, TEXT("Failed to get lobby from list. Ignoring: lobbyIdx=%u, %s; %s"), lobbbyIdx, UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
			continue;
		}

		galaxy::api::Matchmaking()->RequestLobbyData(lobbyID, this);
		err = galaxy::api::GetError();
		if (err)
		{
			UE_LOG_ONLINE(Warning, TEXT("Failed to request lobby data. Ignoring: lobbyID=%llu; %s; %s"), lobbbyIdx, UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
			continue;
		}

		pendingLobbyList.Add(lobbyID);
	}

	if (!pendingLobbyList.Num())
	{
		UE_LOG_ONLINE(Error, TEXT("Lobby list retrieved, yet no lobby data requested"));
		return false;
	}

	searchSettings->SearchResults.Empty(pendingLobbyList.Num());
	return true;
}

void FRequestLobbyListListener::OnLobbyDataRetrieveSuccess(const galaxy::api::GalaxyID& InLobbyID)
{
	UE_LOG_ONLINE(Display, TEXT("FRequestLobbyListListener::OnLobbyDataRetrieveSuccess()"), InLobbyID.ToUint64());

	verifyf(pendingLobbyList.RemoveSwap(InLobbyID) > 0, TEXT("Unknown lobby (lobbyID=%llu). This shall never happen. Please contact GalaxySDK team"), InLobbyID.ToUint64());

	FOnlineSessionSearchResult newSearchResult;
	if (!OnlineSessionUtils::Fill(InLobbyID, newSearchResult))
	{
		UE_LOG_ONLINE(Error, TEXT("Failed to create Session data: lobbyID=%llu"), InLobbyID.ToUint64());
		TriggerOnFindSessionsCompleteDelegates(false);
		return;
	}

	if (TestFilters(postOperationSearchQueryParams, newSearchResult))
		searchSettings->SearchResults.Emplace(MoveTemp(newSearchResult));

	if (pendingLobbyList.Num() == 0)
		TriggerOnFindSessionsCompleteDelegates(true);
}

void FRequestLobbyListListener::OnLobbyDataRetrieveFailure(const galaxy::api::GalaxyID& InLobbyID, galaxy::api::ILobbyDataRetrieveListener::FailureReason InFailureReason)
{
	UE_LOG_ONLINE(Display, TEXT("OnLobbyDataRetrieveFailure, lobbyID=%llu"), InLobbyID.ToUint64());

	verifyf(pendingLobbyList.RemoveSwap(InLobbyID) > 0, TEXT("Unknown lobby (lobbyID=%llu). This shall never happen. Please contact GalaxySDK team"), InLobbyID.ToUint64());

	switch (InFailureReason)
	{
		case galaxy::api::ILobbyDataRetrieveListener::FAILURE_REASON_LOBBY_DOES_NOT_EXIST:
		{
			UE_LOG_ONLINE(Error, TEXT("Failed to get lobby data. Lobby does not exists: lobbyID=%llu"), InLobbyID.ToUint64());
			pendingLobbyList.RemoveSwap(InLobbyID);

			if (pendingLobbyList.Num() == 0)
				TriggerOnFindSessionsCompleteDelegates(true);

			// Continue waiting for other lobbies data
			return;
		}
		case galaxy::api::ILobbyDataRetrieveListener::FAILURE_REASON_UNDEFINED:
		default:
			UE_LOG_ONLINE(Error, TEXT("Unknown failure when retrieving lobby data: lobbyID=%llu"), InLobbyID.ToUint64());
	}

	TriggerOnFindSessionsCompleteDelegates(false);
}
