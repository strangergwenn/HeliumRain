#include "OnlineSessionGOG.h"

#include "LobbyData.h"
#include "CreateLobbyListener.h"
#include "RequestLobbyListListener.h"
#include "JoinLobbyListener.h"
#include "LobbyStartListener.h"
#include "GameInvitationReceivedListener.h"
#include "GameInvitationAcceptedListener.h"
#include "GetSessionDetailsListener.h"
#include "UpdateLobbyListener.h"
#include "Types/UrlGOG.h"
#include "Types/UserOnlineAccountGOG.h"
#include "Converters/NamedVariantDataConverter.h"
#include "OnlineSessionUtils.h"
#include "VariantDataUtils.h"

#include "Online.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineLeaderboardInterface.h"
#include "NumericLimits.h"

#include <algorithm>

namespace
{

	constexpr auto TEXT_SESSION_NAME_OPTION = TEXT("SessionName");

	void FlushLeaderboards(IOnlineSubsystem& InSubsystem, FName InSessionName)
	{
		auto onlineLeaderboardsInterface = InSubsystem.GetLeaderboardsInterface();
		if (!onlineLeaderboardsInterface.IsValid())
		{
			UE_LOG_ONLINE(Warning, TEXT("Failed to flush leaderboards: NULL leaderboards interface"));
			return;
		}

		onlineLeaderboardsInterface->FlushLeaderboards(InSessionName);
	}

	bool ConvertToLobbySearchComparisonType(EOnlineComparisonOp::Type InComparisonType, galaxy::api::LobbyComparisonType& OutLobbyComparisonType)
	{
		switch (InComparisonType)
		{
			case EOnlineComparisonOp::Equals:
			{
				OutLobbyComparisonType = galaxy::api::LOBBY_COMPARISON_TYPE_EQUAL;
				return true;
			}
			case EOnlineComparisonOp::NotEquals:
			{
				OutLobbyComparisonType = galaxy::api::LOBBY_COMPARISON_TYPE_NOT_EQUAL;
				return true;
			}
			case EOnlineComparisonOp::GreaterThan:
			{
				OutLobbyComparisonType = galaxy::api::LOBBY_COMPARISON_TYPE_GREATER;
				return true;
			}
			case EOnlineComparisonOp::GreaterThanEquals:
			{
				OutLobbyComparisonType = galaxy::api::LOBBY_COMPARISON_TYPE_GREATER_OR_EQUAL;
				return true;
			}
			case EOnlineComparisonOp::LessThan:
			{
				OutLobbyComparisonType = galaxy::api::LOBBY_COMPARISON_TYPE_LOWER;
				return true;
			}
			case EOnlineComparisonOp::LessThanEquals:
			{
				OutLobbyComparisonType = galaxy::api::LOBBY_COMPARISON_TYPE_LOWER_OR_EQUAL;
				return true;
			}
			case EOnlineComparisonOp::Near:
			case EOnlineComparisonOp::In:
			case EOnlineComparisonOp::NotIn:
			default:
				UE_LOG_ONLINE(Error, TEXT("Unsupported comparison operation: type=%s"), EOnlineComparisonOp::ToString(InComparisonType));
		}

		return false;
	}

	template<typename IntType, typename = std::enable_if_t<std::is_integral<IntType>::value>>
	void ApplyAsNumericalFilter(const FSearchParams::ElementType& InSearchQueryParam)
	{
		int32 filterValue;
		if (!GetInt32ValueFromType<IntType>(InSearchQueryParam.Value.Data, filterValue))
		{
			UE_LOG_ONLINE(Warning, TEXT("Numerical value for session search query param is out of int32 range. Skipping: searchParamName=%s, %s"),
				*InSearchQueryParam.Key.ToString(), *InSearchQueryParam.Value.ToString());

			return;
		}

		const auto filter = NamedVariantDataConverter::ToLobbyDataEntry(InSearchQueryParam.Key, InSearchQueryParam.Value.Data);

		if (InSearchQueryParam.Value.ComparisonOp == EOnlineComparisonOp::Near)
		{
			galaxy::api::Matchmaking()->AddRequestLobbyListNearValueFilter(TCHAR_TO_UTF8(*filter.Key), filterValue);
			auto err = galaxy::api::GetError();
			if (err)
			{
				UE_LOG_ONLINE(Warning, TEXT("Failed add request lobby filter near value: searchQueryParamName=%s, %d; %s; %s"),
					*filter.Key, filterValue, ANSI_TO_TCHAR(err->GetName()), ANSI_TO_TCHAR(err->GetMsg()));

				return;
			}
		}

		galaxy::api::LobbyComparisonType lobbyComparisonType;
		if (!ConvertToLobbySearchComparisonType(InSearchQueryParam.Value.ComparisonOp, lobbyComparisonType))
		{
			UE_LOG_ONLINE(Warning, TEXT("Unsupported comparison operation for session search query. Skipping: searchQueryParamName=%s, %s"),
				*InSearchQueryParam.Key.ToString(), *InSearchQueryParam.Value.ToString());

			return;
		}

		galaxy::api::Matchmaking()->AddRequestLobbyListNumericalFilter(TCHAR_TO_UTF8(*filter.Key), filterValue, lobbyComparisonType);
		auto err = galaxy::api::GetError();
		if (err)
		{
			UE_LOG_ONLINE(Warning, TEXT("Failed add request lobby filter for numerical value: searchQueryParamName=%s, %d; %s: %s"),
				*filter.Key, filterValue, ANSI_TO_TCHAR(err->GetName()), ANSI_TO_TCHAR(err->GetMsg()));

			return;
		}

		return;
	}

	void ApplyAsStringFilter(const FSearchParams::ElementType& InSearchQueryParam)
	{
		const auto& filter = NamedVariantDataConverter::ToLobbyDataEntry(InSearchQueryParam.Key, InSearchQueryParam.Value.Data);
		if (filter.Value.IsEmpty())
		{
			UE_LOG_ONLINE(Display, TEXT("Empty filter. Skipping: searchQueryParamName=%s, %s"),
				*InSearchQueryParam.Key.ToString(), *InSearchQueryParam.Value.ToString());
			return;
		}

		galaxy::api::LobbyComparisonType lobbyComparisonType;
		if (!ConvertToLobbySearchComparisonType(InSearchQueryParam.Value.ComparisonOp, lobbyComparisonType))
		{
			UE_LOG_ONLINE(Warning, TEXT("Unsupported comparison operation for session search query. Skipping: searchQueryParamName=%s, %s"),
				*InSearchQueryParam.Key.ToString(), *InSearchQueryParam.Value.ToString());

			return;
		}

		galaxy::api::Matchmaking()->AddRequestLobbyListStringFilter(TCHAR_TO_UTF8(*filter.Key), TCHAR_TO_UTF8(*filter.Value), lobbyComparisonType);
		auto err = galaxy::api::GetError();
		if (err)
			UE_LOG_ONLINE(Warning, TEXT("Failed add request lobby filter for string value: searchQueryParamName=%s, %s; %s: %s"),
				*filter.Key, *filter.Value, ANSI_TO_TCHAR(err->GetName()), ANSI_TO_TCHAR(err->GetMsg()));
	}

	bool ParseAndApplyPredefinedFilter(const FSearchParams::ElementType& InSearchQueryParam, FSearchParams& OutPostOperationSearchQueryParams)
	{
		// See OnlineSessionSettings.h for the full list of predefined settings

		const auto& key = InSearchQueryParam.Key;

		if (key == SEARCH_EMPTY_SERVERS_ONLY
			|| key == SEARCH_NONEMPTY_SERVERS_ONLY
			|| key == SEARCH_MINSLOTSAVAILABLE
			|| key == SEARCH_USER)
		{
			OutPostOperationSearchQueryParams.Emplace(InSearchQueryParam.Key, InSearchQueryParam.Value);
			return true;
		}
		else if (key == SEARCH_DEDICATED_ONLY
			|| key == SEARCH_SECURE_SERVERS_ONLY
			|| key == SEARCH_PRESENCE
			|| key == SEARCH_EXCLUDE_UNIQUEIDS)
		{
			UE_LOG_ONLINE(Warning, TEXT("Search param is not supported. Skipping: searchParamName=%s"), *key.ToString());
			return true;
		}

		// Pass the rest of the settings as strings. Including the following: SEARCH_KEYWORDS, SEARCH_SWITCH_SELECTION_METHOD, SEARCH_XBOX_LIVE_SESSION_TEMPLATE_NAME, SEARCH_XBOX_LIVE_HOPPER_NAME
		return false;
	}

	void ParseAndApplyLobbySearchFilters(const FOnlineSearchSettings& InSearchSettings, FSearchParams& OutPostOperationSearchQueryParams)
	{
		for (const auto& searchQueryParam : InSearchSettings.SearchParams)
		{
			if (ParseAndApplyPredefinedFilter(searchQueryParam, OutPostOperationSearchQueryParams))
				continue;

			switch (searchQueryParam.Value.Data.GetType())
			{
				case EOnlineKeyValuePairDataType::Empty:
					continue;

				case EOnlineKeyValuePairDataType::Bool:
				case EOnlineKeyValuePairDataType::String:
				{
					ApplyAsStringFilter(searchQueryParam);
					continue;
				}

				case EOnlineKeyValuePairDataType::UInt64:
				{
					ApplyAsNumericalFilter<uint64>(searchQueryParam);
					continue;
				}
				case EOnlineKeyValuePairDataType::Int64:
				{
					ApplyAsNumericalFilter<int64>(searchQueryParam);
					continue;
				}
				case EOnlineKeyValuePairDataType::UInt32:
				{
					ApplyAsNumericalFilter<uint32>(searchQueryParam);
					continue;
				}
				case EOnlineKeyValuePairDataType::Int32:
				{
					ApplyAsNumericalFilter<int32>(searchQueryParam);
					continue;
				}

				case EOnlineKeyValuePairDataType::Double:
				case EOnlineKeyValuePairDataType::Float:
				case EOnlineKeyValuePairDataType::Blob:
				default:
				{
					UE_LOG_ONLINE(Warning, TEXT("Unsupported session search param type. Skipping: searchParamName=%s, %s"),
						*searchQueryParam.Key.ToString(), *searchQueryParam.Value.ToString());
					continue;
				}
			}
		}
	}

	galaxy::api::LobbyType GetLobbyType(const FOnlineSessionSettings& InSessionSettings)
	{
		if (InSessionSettings.bShouldAdvertise)
			return galaxy::api::LOBBY_TYPE_PUBLIC;

		if (InSessionSettings.bAllowJoinViaPresenceFriendsOnly)
			return galaxy::api::LOBBY_TYPE_FRIENDS_ONLY;

		return galaxy::api::LOBBY_TYPE_PRIVATE;
	}

}

FOnlineSessionGOG::FOnlineSessionGOG(IOnlineSubsystem& InSubsystem, TSharedRef<class FUserOnlineAccountGOG> InUserOnlineAccount)
	: subsystemGOG{InSubsystem}
	, ownUserOnlineAccount{MoveTemp(InUserOnlineAccount)}
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::ctor()"));
}

FOnlineSessionGOG::~FOnlineSessionGOG()
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::dtor()"));

	for (const auto& session : storedSessions)
	{
		if (session.SessionInfo.IsValid() && session.SessionInfo->IsValid())
			galaxy::api::Matchmaking()->LeaveLobby(FUniqueNetIdGOG{session.SessionInfo->GetSessionId()});
	}
}

bool FOnlineSessionGOG::CreateSession(int32 InHostingPlayerNum, FName InSessionName, const FOnlineSessionSettings& InSessionSettings)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::CreateSession: sessionName='%s'"), *InSessionName.ToString());

	CheckLocalUserNum(InHostingPlayerNum);

	if (InSessionSettings.NumPublicConnections < 0)
	{
		UE_LOG_ONLINE(Error, TEXT("Invalid number of public connections"));
		return false;
	}

	if (InSessionSettings.NumPrivateConnections > 0)
	{
		UE_LOG_ONLINE(Warning, TEXT("Private connections are not supported for GOG sessions"));
	}

	if (InSessionSettings.bIsLANMatch)
		UE_LOG_ONLINE(Warning, TEXT("LAN matches are not supported on GOG platform"));

	if (InSessionSettings.bIsDedicated)
		UE_LOG_ONLINE(Warning, TEXT("Dedicated servers are not implemented yet by GOG platform"));

	if (GetNamedSession(InSessionName) != nullptr)
	{
		UE_LOG_ONLINE(Warning, TEXT("Cannot create the same session twice: sessionName='%s'"), *InSessionName.ToString());
		TriggerOnCreateSessionCompleteDelegates(InSessionName, false);
		return false;
	}

	auto listener = CreateListener<FCreateLobbyListener>(
		*this,
		InSessionName,
		ownUserOnlineAccount->GetUserId(),
		ownUserOnlineAccount->GetDisplayName(),
		InSessionSettings);

	galaxy::api::Matchmaking()->CreateLobby(
		GetLobbyType(InSessionSettings),
		InSessionSettings.NumPublicConnections,
		false,
		galaxy::api::LOBBY_TOPOLOGY_TYPE_STAR,
		listener.Value,
		listener.Value);

	auto err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to create lobby[0]: %s; %s"), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

		FreeListener(MoveTemp(listener.Key));

		TriggerOnCreateSessionCompleteDelegates(InSessionName, false);
		return false;
	}

	return true;
}

bool FOnlineSessionGOG::CreateSession(const FUniqueNetId& InHostingPlayerId, FName InSessionName, const FOnlineSessionSettings& InSessionSettings)
{
	return CreateSession(LOCAL_USER_NUM, MoveTemp(InSessionName), InSessionSettings);
}

FNamedOnlineSession* FOnlineSessionGOG::AddNamedSession(FName InSessionName, const FOnlineSessionSettings& InSessionSettings)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::AddNamedSession('%s')"), *InSessionName.ToString());

	return CreateNamedSession(MoveTemp(InSessionName), InSessionSettings);
}

FNamedOnlineSession* FOnlineSessionGOG::AddNamedSession(FName InSessionName, const FOnlineSession& InSession)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::AddNamedSession('%s')"), *InSessionName.ToString());

	return CreateNamedSession(MoveTemp(InSessionName), InSession);
}

FNamedOnlineSession* FOnlineSessionGOG::GetNamedSession(FName InSessionName)
{
	return const_cast<FNamedOnlineSession*>(static_cast<const FOnlineSessionGOG*>(this)->GetNamedSession(MoveTemp(InSessionName)));
}

const FNamedOnlineSession* FOnlineSessionGOG::GetNamedSession(FName InSessionName) const
{
	UE_LOG_ONLINE(VeryVerbose, TEXT("FOnlineSessionGOG::GetNamedSession('%s')"), *InSessionName.ToString());

	return storedSessions.FindByPredicate([&](const auto& session) {
		return session.SessionName == InSessionName;
	});
}

void FOnlineSessionGOG::RemoveNamedSession(FName InSessionName)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::RemoveNamedSession('%s')"), *InSessionName.ToString());

	storedSessions.RemoveAllSwap([&](const auto& session) {
		return session.SessionName == InSessionName;
	});
}

bool FOnlineSessionGOG::HasPresenceSession()
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::HasPresenceSession()"));

	return storedSessions.FindByPredicate([&](auto& session) {
		return session.SessionSettings.bUsesPresence;
	}) != nullptr;
}

EOnlineSessionState::Type FOnlineSessionGOG::GetSessionState(FName InSessionName) const
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::GetSessionState('%s')"), *InSessionName.ToString());

	auto storedSession = GetNamedSession(InSessionName);
	if (!storedSession)
		return EOnlineSessionState::NoSession;

	return storedSession->SessionState;
}

bool FOnlineSessionGOG::StartSession(FName InSessionName)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::StartSession('%s')"), *InSessionName.ToString());

	FNamedOnlineSession* storedSession = GetNamedSession(InSessionName);
	if (storedSession == nullptr)
	{
		UE_LOG_ONLINE(Error, TEXT("Cannot start a session as it does not exist: sessionName='%s'"), *InSessionName.ToString());

		TriggerOnStartSessionCompleteDelegates(InSessionName, false);
		return false;
	}

	if (storedSession->SessionState == EOnlineSessionState::Starting)
	{
		UE_LOG_ONLINE(Warning, TEXT("Session is already marked as starting. Skipping: sessionName='%s'"), *InSessionName.ToString());
		// Delegate will be triggered by the initial call
		return false;
	}

	if (storedSession->SessionState != EOnlineSessionState::Pending
		&& storedSession->SessionState != EOnlineSessionState::Ended)
	{
		UE_LOG_ONLINE(Warning, TEXT("Cannot start a session in current state: sessionName='%s', state='%s'"), *InSessionName.ToString(), EOnlineSessionState::ToString(storedSession->SessionState));

		TriggerOnStartSessionCompleteDelegates(InSessionName, false);
		return false;
	}

	if (!storedSession->SessionInfo.IsValid() || !storedSession->SessionInfo->IsValid())
	{
		UE_LOG_ONLINE(Warning, TEXT("Ivalid session info"));
		TriggerOnStartSessionCompleteDelegates(InSessionName, false);
		return false;
	}

	storedSession->SessionState = EOnlineSessionState::Starting;

	galaxy::api::GalaxyID lobbyID = FUniqueNetIdGOG{storedSession->SessionInfo->GetSessionId()};

	auto lobbyJoinable = galaxy::api::Matchmaking()->IsLobbyJoinable(lobbyID);
	auto err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to get lobby joinability: sessionName='%s', lobbyID=%llu, %s: %s"),
			*InSessionName.ToString(), lobbyID.ToUint64(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

		storedSession->SessionState = EOnlineSessionState::Pending;
		TriggerOnStartSessionCompleteDelegates(InSessionName, false);
		return false;
	}

	if (lobbyJoinable == storedSession->SessionSettings.bAllowJoinInProgress)
	{
		storedSession->SessionState = EOnlineSessionState::InProgress;
		TriggerOnStartSessionCompleteDelegates(InSessionName, true);
		return true;
	}

	auto listener = CreateListener<FLobbyStartListener>(*this, lobbyID, InSessionName, storedSession->SessionSettings.bAllowJoinInProgress);

	galaxy::api::Matchmaking()->SetLobbyJoinable(lobbyID, storedSession->SessionSettings.bAllowJoinInProgress, listener.Value);
	err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to change lobby joinability: lobbyID=%llu, %s: %s"),
			lobbyID.ToUint64(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

		FreeListener(MoveTemp(listener.Key));
		storedSession->SessionState = EOnlineSessionState::Pending;
		TriggerOnStartSessionCompleteDelegates(InSessionName, false);
		return false;
	}

	return true;
}

bool FOnlineSessionGOG::UpdateSession(FName InSessionName, FOnlineSessionSettings& InUpdatedSessionSettings, bool InShouldRefreshOnlineData)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::UpdateSession('%s')"), *InSessionName.ToString());

	auto* storedSession = GetNamedSession(InSessionName);
	if (!storedSession)
	{
		UE_LOG_ONLINE(Warning, TEXT("Cannot update a session that does not exists: sessionName='%s'"), *InSessionName.ToString());
		TriggerOnUpdateSessionCompleteDelegates(InSessionName, false);
		return false;
	}

	if (*storedSession->OwningUserId != *ownUserOnlineAccount->GetUserId())
	{
		UE_LOG_ONLINE(Warning, TEXT("Cannot update session. Player is not a session owner: sessionName='%s', sessionOwnerID='%s', userID='%s'"),
			*InSessionName.ToString(), *storedSession->OwningUserId->ToString(), *ownUserOnlineAccount->GetUserId()->ToString());
		TriggerOnUpdateSessionCompleteDelegates(InSessionName, false);
		return false;
	}

	if (!InShouldRefreshOnlineData)
	{
		UE_LOG_ONLINE(VeryVerbose, TEXT("Received local session update"));
		storedSession->SessionSettings = InUpdatedSessionSettings;
		TriggerOnUpdateSessionCompleteDelegates(InSessionName, true);
		return true;
	}

	FUniqueNetIdGOG sessionID{storedSession->SessionInfo->GetSessionId()};

	InUpdatedSessionSettings.Settings.Emplace(lobby_data::SESSION_OWNER_NAME, FOnlineSessionSetting{ownUserOnlineAccount->GetDisplayName(), EOnlineDataAdvertisementType::ViaOnlineService});
	InUpdatedSessionSettings.Settings.Emplace(lobby_data::SESSION_OWNER_ID, FOnlineSessionSetting{ownUserOnlineAccount->GetUserId()->ToString(), EOnlineDataAdvertisementType::ViaOnlineService});

	auto sessionSettingsToUpdate = InUpdatedSessionSettings;
	TSet<FString> sessionSettingsToDelete;

	const auto& storedSessionSettings = storedSession->SessionSettings.Settings;
	for (const auto& storedSetting : storedSessionSettings)
	{
		auto updatedSetting = sessionSettingsToUpdate.Settings.Find(storedSetting.Key);
		if (!updatedSetting)
		{
			auto convertedSettingName = NamedVariantDataConverter::ToLobbyDataEntry(storedSetting.Key, storedSetting.Value.Data).Key;
			sessionSettingsToDelete.Emplace(MoveTemp(convertedSettingName));
			continue;
		}

		if (updatedSetting->Data == storedSetting.Value.Data)
		{
			sessionSettingsToUpdate.Remove(storedSetting.Key);
			continue;
		}
	}

	auto updatedLobbyType = GetLobbyType(sessionSettingsToUpdate);
	auto shouldAdvertiseViaPresence = OnlineSessionUtils::ShouldAdvertiseViaPresence(InUpdatedSessionSettings);

	if (sessionSettingsToUpdate.Settings.Num() == 0
		&& sessionSettingsToDelete.Num() == 0
		&& updatedLobbyType == GetLobbyType(storedSession->SessionSettings)
		&& sessionSettingsToUpdate.NumPublicConnections == storedSession->SessionSettings.NumPublicConnections
		&& shouldAdvertiseViaPresence == OnlineSessionUtils::ShouldAdvertiseViaPresence(storedSession->SessionSettings)
		&& (storedSession->SessionState != EOnlineSessionState::InProgress
			|| InUpdatedSessionSettings.bAllowJoinInProgress == storedSession->SessionSettings.bAllowJoinInProgress))
	{
		UE_LOG_ONLINE(Display, TEXT("No changes in Session settings"));
		TriggerOnUpdateSessionCompleteDelegates(InSessionName, true);
		return true;
	}

	if (!OnlineSessionUtils::SetLobbyData(sessionID, sessionSettingsToUpdate)
		|| !OnlineSessionUtils::DeleteLobbyData(sessionID, sessionSettingsToDelete))
	{
		TriggerOnUpdateSessionCompleteDelegates(InSessionName, false);
		return false;
	}

	galaxy::api::Matchmaking()->SetLobbyType(sessionID, updatedLobbyType);
	auto err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to update lobby type: %s: %s"), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

		TriggerOnUpdateSessionCompleteDelegates(InSessionName, false);
		return false;
	}

	galaxy::api::Matchmaking()->SetMaxNumLobbyMembers(sessionID, sessionSettingsToUpdate.NumPublicConnections);
	err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Error, TEXT("Failed to update lobby maximum size: lobbyID='%s'; %s: %s"),
			*sessionID.ToString(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
		TriggerOnUpdateSessionCompleteDelegates(InSessionName, false);
		return false;
	}

	if (shouldAdvertiseViaPresence)
	{
		FString connectString;
		if (!GetResolvedConnectString(InSessionName, connectString))
			return false;

		// Ignore rich presence update result as it seems to be not crucial
		galaxy::api::Friends()->SetRichPresence("connect", TCHAR_TO_UTF8(*connectString));
		err = galaxy::api::GetError();
		if (err)
		{
			UE_LOG_ONLINE(Error, TEXT("Failed to set rich presence connect string: connectString='%s'; %s: %s"),
				UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
		}
	}

	// Setting all lobby data is done in bulk, so it's enough to listen only to the last operation
	auto listener = CreateListener<FUpdateLobbyListener>(*this, InSessionName);

	bool isLobbyJoinable = storedSession->SessionState != EOnlineSessionState::InProgress
		|| InUpdatedSessionSettings.bAllowJoinInProgress;

	galaxy::api::Matchmaking()->SetLobbyJoinable(sessionID, isLobbyJoinable, listener.Value);
	err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to change lobby joinability: lobbyID='%s'; %s: %s"),
			*sessionID.ToString(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

		FreeListener(MoveTemp(listener.Key));

		TriggerOnUpdateSessionCompleteDelegates(InSessionName, false);
		return false;
	}

	return true;
}

bool FOnlineSessionGOG::EndSession(FName InSessionName)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::EndSession('%s')"), *InSessionName.ToString());

	auto* storedSession = GetNamedSession(InSessionName);
	if (!storedSession)
	{
		UE_LOG_ONLINE(Warning, TEXT("Cannot end a session that does not exists: sessionName='%s'"), *InSessionName.ToString());
		TriggerOnEndSessionCompleteDelegates(InSessionName, false);
		return false;
	}

	if (storedSession->SessionState != EOnlineSessionState::InProgress)
	{
		UE_LOG_ONLINE(Warning, TEXT("Cannot end a session that was not started: sessionName='%s', state='%s'"), *InSessionName.ToString(), EOnlineSessionState::ToString(storedSession->SessionState));
		TriggerOnEndSessionCompleteDelegates(InSessionName, false);
		return false;
	}

	storedSession->SessionState = EOnlineSessionState::Ending;

	FlushLeaderboards(subsystemGOG, InSessionName);

	storedSession->SessionState = EOnlineSessionState::Ended;

	TriggerOnEndSessionCompleteDelegates(InSessionName, true);
	return true;
}

bool FOnlineSessionGOG::DestroySession(FName InSessionName, const FOnDestroySessionCompleteDelegate& InCompletionDelegate)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::DestroySession('%s')"), *InSessionName.ToString());

	FNamedOnlineSession* storedSession = GetNamedSession(InSessionName);
	if (!storedSession)
	{
		UE_LOG_ONLINE(Warning, TEXT("Trying to destroy a session that does not exist: sessionName='%s'"), *InSessionName.ToString());
		InCompletionDelegate.ExecuteIfBound(InSessionName, false);
		TriggerOnDestroySessionCompleteDelegates(InSessionName, false);
		return false;
	}

	if (storedSession->SessionState == EOnlineSessionState::Destroying)
	{
		UE_LOG_ONLINE(Warning, TEXT("Session is already marked to be destroyed. Skipping: sessionName='%s'"), *InSessionName.ToString());
		InCompletionDelegate.ExecuteIfBound(InSessionName, false);
		// Initial operation will trigger global delegate
		return false;
	}

	if (storedSession->SessionState == EOnlineSessionState::Starting
		|| storedSession->SessionState == EOnlineSessionState::InProgress)
	{
		UE_LOG_ONLINE(Warning, TEXT("Destroying a '%s' session: sessionName='%s'"), EOnlineSessionState::ToString(storedSession->SessionState), *InSessionName.ToString());
	}

	storedSession->SessionState = EOnlineSessionState::Destroying;

	FlushLeaderboards(subsystemGOG, InSessionName);

	if (!storedSession->SessionInfo.IsValid()
		|| !storedSession->SessionInfo->IsValid())
	{
		UE_LOG_ONLINE(Warning, TEXT("Invalid session info"));
		InCompletionDelegate.ExecuteIfBound(InSessionName, false);
		TriggerOnDestroySessionCompleteDelegates(InSessionName, false);
		return false;
	}

	galaxy::api::Friends()->DeleteRichPresence("connect");
	auto err = galaxy::api::GetError();
	if (err)
		UE_LOG_ONLINE(Error, TEXT("Failed to delete players connect presence informatio: %s; %s"), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

	galaxy::api::Matchmaking()->LeaveLobby(FUniqueNetIdGOG{storedSession->SessionInfo->GetSessionId()});
	err = galaxy::api::GetError();
	if (err)
		UE_LOG_ONLINE(Error, TEXT("Failed to leave lobby: lobbyID=%llu, %s; %s"), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

	RemoveNamedSession(InSessionName);

	InCompletionDelegate.ExecuteIfBound(InSessionName, true);
	TriggerOnDestroySessionCompleteDelegates(InSessionName, true);
	return true;
}

bool FOnlineSessionGOG::IsPlayerInSession(FName InSessionName, const FUniqueNetId& InUniqueId)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::IsPlayerInSession('%s', '%s')"), *InSessionName.ToString(), *InUniqueId.ToString());

	auto storedSession = GetNamedSession(InSessionName);
	if (!storedSession)
	{
		UE_LOG_ONLINE(Error, TEXT("Session not found"));
		return false;
	}

	return storedSession->RegisteredPlayers.ContainsByPredicate([&](const auto& RegisterePlayer) {
		return *RegisterePlayer == InUniqueId;
	});
}

bool FOnlineSessionGOG::StartMatchmaking(const TArray<TSharedRef<const FUniqueNetId>>&, FName InSessionName, const FOnlineSessionSettings&, TSharedRef<FOnlineSessionSearch>&)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::StartMatchmaking('%s')"), *InSessionName.ToString());

	UE_LOG_ONLINE(Error, TEXT("Matchmaking is not implemented yet"));
	TriggerOnMatchmakingCompleteDelegates(InSessionName, false);

	return false;
}

bool FOnlineSessionGOG::CancelMatchmaking(int32, FName InSessionName)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::CancelMatchmaking('%s')"), *InSessionName.ToString());

	UE_LOG_ONLINE(Error, TEXT("Matchmaking is not implemented yet"));
	TriggerOnCancelMatchmakingCompleteDelegates(InSessionName, false);

	return false;
}

bool FOnlineSessionGOG::CancelMatchmaking(const FUniqueNetId& /*InSearchingPlayerId*/, FName InsessionName)
{
	return CancelMatchmaking(LOCAL_USER_NUM, InsessionName);
}

bool FOnlineSessionGOG::FindSessions(int32 InSearchingPlayerNum, const TSharedRef<FOnlineSessionSearch>& InOutSearchSettings)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::FindSessions()"));

	CheckLocalUserNum(InSearchingPlayerNum);

	if (InOutSearchSettings->SearchState == EOnlineAsyncTaskState::InProgress)
	{
		UE_LOG_ONLINE(Warning, TEXT("A session search is already pending. Ignoring this request"));
		// Delegate will be triggered by initial call
		return false;
	}

	if (InOutSearchSettings->bIsLanQuery)
		UE_LOG_ONLINE(Warning, TEXT("LAN is not supported by GOG platform. Ignoring this value"));

	if (InOutSearchSettings->TimeoutInSeconds > 0)
		UE_LOG_ONLINE(Warning, TEXT("Search timeout is not supported. Ignoring this value"));

	if (InOutSearchSettings->PingBucketSize > 0)
		UE_LOG_ONLINE(Warning, TEXT("Ping-based search is not supported. Ignoring this value"));

	if (InOutSearchSettings->MaxSearchResults > 0)
	{
		galaxy::api::Matchmaking()->AddRequestLobbyListResultCountFilter(static_cast<uint32_t>(InOutSearchSettings->MaxSearchResults));
		auto err = galaxy::api::GetError();
		if (err)
		{
			UE_LOG_ONLINE(Warning, TEXT("Failed add request lobby count filter: %s: %s"), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

			InOutSearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
			TriggerOnFindSessionsCompleteDelegates(false);
			return false;
		}
	}
	else
		UE_LOG_ONLINE(Warning, TEXT("Invalid number of max search results. Ignoring this value"));

	FSearchParams postOperationSearchQueryParams;
	ParseAndApplyLobbySearchFilters(InOutSearchSettings->QuerySettings, postOperationSearchQueryParams);

	auto listener = CreateListener<FRequestLobbyListListener>(*this, InOutSearchSettings, MoveTemp(postOperationSearchQueryParams));

	galaxy::api::Matchmaking()->RequestLobbyList(false, listener.Value);
	auto err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to request lobby list: %s; %s"), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

		FreeListener(MoveTemp(listener.Key));

		InOutSearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
		TriggerOnFindSessionsCompleteDelegates(false);
		return false;
	}

	InOutSearchSettings->SearchState = EOnlineAsyncTaskState::InProgress;

	return true;
}

bool FOnlineSessionGOG::FindSessions(const FUniqueNetId& /*InSearchingPlayerId*/, const TSharedRef<FOnlineSessionSearch>& InOutSearchSettings)
{
	return FindSessions(LOCAL_USER_NUM, InOutSearchSettings);
}

bool FOnlineSessionGOG::FindSessionById(
	const FUniqueNetId& /*InSearchingUserId*/,
	const FUniqueNetId& InSessionId,
	const FUniqueNetId& InFriendId,
	const FOnSingleSessionResultCompleteDelegate& InCompletionDelegate)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::FindSessionById: sessionID='%s'"), *InSessionId.ToString());

	FUniqueNetIdGOG sessionID{InSessionId};
	if (!sessionID.IsValid() || !sessionID.IsLobby())
	{
		UE_LOG_ONLINE(Display, TEXT("Invalid SessionID: sessionID='%s'"), *InSessionId.ToString());
		InCompletionDelegate.ExecuteIfBound(LOCAL_USER_NUM, false, {});
		return false;
	}

	auto listener = CreateListener<FGetSessionDetailsListener>(*this, sessionID, InFriendId, InCompletionDelegate);

	galaxy::api::Matchmaking()->RequestLobbyData(sessionID, listener.Value);
	auto err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to request session data: sessionID='%s'; %s: %s"),
			*InSessionId.ToString(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

		FreeListener(MoveTemp(listener.Key));

		InCompletionDelegate.ExecuteIfBound(LOCAL_USER_NUM, false, {});
		return false;
	}

	return true;
}

bool FOnlineSessionGOG::CancelFindSessions()
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::CancelFindSessions()"));

	UE_LOG_ONLINE(Error, TEXT("CancelFindSessions is not implemented yet"));

	TriggerOnCancelFindSessionsCompleteDelegates(false);
	return false;
}

bool FOnlineSessionGOG::PingSearchResults(const FOnlineSessionSearchResult& InSearchResult)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::PingSearchResults('%s')"), *InSearchResult.GetSessionIdStr());

	UE_LOG_ONLINE(Error, TEXT("PingSearchResults is not avaialable yet on GOG platform"));

	TriggerOnPingSearchResultsCompleteDelegates(false);
	return false;
}

bool FOnlineSessionGOG::JoinSession(int32 InPlayerNum, FName InSessionName, const FOnlineSessionSearchResult& InDesiredSession)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::JoinSession('%s', '%s')"), *InSessionName.ToString(), *InDesiredSession.GetSessionIdStr());

	if (GetNamedSession(InSessionName) != nullptr)
	{
		UE_LOG_ONLINE(Warning, TEXT("Cannot join a session twice: joiningSession='%s'"), *InSessionName.ToString());
		TriggerOnJoinSessionCompleteDelegates(InSessionName, EOnJoinSessionCompleteResult::AlreadyInSession);
		return false;
	}

	if (!InDesiredSession.IsValid())
	{
		UE_LOG_ONLINE(Error, TEXT("Invalid session"));
		TriggerOnJoinSessionCompleteDelegates(InSessionName, EOnJoinSessionCompleteResult::CouldNotRetrieveAddress);
		return false;
	}

	const auto& sessionID = InDesiredSession.Session.SessionInfo->GetSessionId();

	auto listener = CreateListener<FJoinLobbyListener>(*this, InSessionName, InDesiredSession.Session);

	galaxy::api::Matchmaking()->JoinLobby(FUniqueNetIdGOG{sessionID}, listener.Value);
	auto err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to join lobby: lobbyID=%s; %s; %s"), *sessionID.ToString(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
		TriggerOnJoinSessionCompleteDelegates(InSessionName, EOnJoinSessionCompleteResult::UnknownError);

		FreeListener(MoveTemp(listener.Key));

		return false;
	}

	return true;
}

bool FOnlineSessionGOG::JoinSession(const FUniqueNetId& /*InPlayerId*/, FName InSessionName, const FOnlineSessionSearchResult& InDesiredSession)
{
	return JoinSession(LOCAL_USER_NUM, InSessionName, InDesiredSession);
}

bool FOnlineSessionGOG::FindFriendSession(int32 InLocalUserNum, const FUniqueNetId&)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::FindFriendSession()"));

	UE_LOG_ONLINE(Error, TEXT("FindFriendSession is not supported by GOG platform"));

	TriggerOnFindFriendSessionCompleteDelegates(InLocalUserNum, false, {});

	return false;
}

bool FOnlineSessionGOG::FindFriendSession(const FUniqueNetId&, const FUniqueNetId&)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::FindFriendSession()"));

	UE_LOG_ONLINE(Error, TEXT("FindFriendSession is not supported by GOG platform"));

	TriggerOnFindFriendSessionCompleteDelegates(LOCAL_USER_NUM, false, {});

	return false;
}

bool FOnlineSessionGOG::FindFriendSession(const FUniqueNetId&, const TArray<TSharedRef<const FUniqueNetId>>&)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::FindFriendSession()"));

	UE_LOG_ONLINE(Error, TEXT("FindFriendSession is not supported by GOG platform"));

	TriggerOnFindFriendSessionCompleteDelegates(LOCAL_USER_NUM, false, {});

	return false;
}

bool FOnlineSessionGOG::SendSessionInviteToFriend(int32 InLocalUserNum, FName InSessionName, const FUniqueNetId& InFriend)
{
	return SendSessionInviteToFriends(InLocalUserNum, InSessionName, {MakeShared<FUniqueNetIdGOG>(InFriend)});
}

bool FOnlineSessionGOG::SendSessionInviteToFriend(const FUniqueNetId& /*InLocalUserId*/, FName InSessionName, const FUniqueNetId& InFriend)
{
	return SendSessionInviteToFriends(LOCAL_USER_NUM, InSessionName, {MakeShared<FUniqueNetIdGOG>(InFriend)});
}

bool FOnlineSessionGOG::SendSessionInviteToFriends(int32 InLocalUserNum, FName InSessionName, const TArray<TSharedRef<const FUniqueNetId>>& InFriends)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::SendSessionInviteToFriends('%s', %d)"), *InSessionName.ToString(), InFriends.Num());

	auto storedSession = GetNamedSession(InSessionName);
	if (!storedSession)
	{
		UE_LOG_ONLINE(Warning, TEXT("Invalid session"));
		return false;
	}

	if (!storedSession->SessionInfo.IsValid() || !storedSession->SessionInfo->IsValid())
	{
		UE_LOG_ONLINE(Warning, TEXT("Invalid session info"));
		return false;
	}

	FString connectString;
	if (!GetResolvedConnectString(InSessionName, connectString))
		return false;

	bool invitationSentSuccessfully = true;
	for (auto invitedFriend : InFriends)
	{
		galaxy::api::Friends()->SendInvitation(FUniqueNetIdGOG{*invitedFriend}, TCHAR_TO_UTF8(*connectString));
		auto err = galaxy::api::GetError();
		if (err)
		{
			UE_LOG_ONLINE(Warning, TEXT("Failed to invite a friend: friendId='%s'; %s: %s"),
				*invitedFriend->ToDebugString(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
			invitationSentSuccessfully = false;
		}
	}

	return invitationSentSuccessfully;
}

bool FOnlineSessionGOG::SendSessionInviteToFriends(const FUniqueNetId& /*InLocalUserId*/, FName InSessionName, const TArray<TSharedRef<const FUniqueNetId>>& InFriends)
{
	return SendSessionInviteToFriends(LOCAL_USER_NUM, InSessionName, InFriends);
}

bool FOnlineSessionGOG::GetResolvedConnectStringFromSession(const FOnlineSession& InSession, FString& OutConnectString) const
{
	if (!InSession.SessionInfo.IsValid()
		|| !InSession.SessionInfo->IsValid())
	{
		UE_LOG_ONLINE(Warning, TEXT("Invalid SessionInfo"));
		return false;
	}

	OutConnectString = FUrlGOG{InSession.SessionInfo->GetSessionId()}.ToString();
	return true;
}

bool FOnlineSessionGOG::GetResolvedConnectString(FName InSessionName, FString& OutConnectInfo, FName)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::GetResolvedConnectString('%s')"), *InSessionName.ToString());

	auto storedSession = GetNamedSession(InSessionName);
	if (!storedSession)
	{
		UE_LOG_ONLINE(Warning, TEXT("Session info not found"));
		return false;
	}

	return GetResolvedConnectStringFromSession(*storedSession, OutConnectInfo);
}

bool FOnlineSessionGOG::GetResolvedConnectString(const FOnlineSessionSearchResult& InSearchResult, const FName, FString& OutConnectInfo)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::GetResolvedConnectString()"));

	if (!InSearchResult.IsValid())
	{
		UE_LOG_ONLINE(Warning, TEXT("Invalid session info"));
		return false;
	}

	return GetResolvedConnectStringFromSession(InSearchResult.Session, OutConnectInfo);
}

FOnlineSessionSettings* FOnlineSessionGOG::GetSessionSettings(FName InSessionName)
{
	UE_LOG_ONLINE(VeryVerbose, TEXT("FOnlineSessionGOG::GetResolvedConnectString('%s')"), *InSessionName.ToString());

	auto storedSession = GetNamedSession(InSessionName);
	if (!storedSession)
	{
		UE_LOG_ONLINE(Warning, TEXT("Session not found"));
		return nullptr;
	}

	return &storedSession->SessionSettings;
}

bool FOnlineSessionGOG::RegisterPlayer(FName InSessionName, const FUniqueNetId& InPlayerID, bool InWasInvited)
{
	return RegisterPlayers(InSessionName, {MakeShared<FUniqueNetIdGOG>(InPlayerID)}, InWasInvited);
}

bool FOnlineSessionGOG::RegisterPlayers(FName InSessionName, const TArray< TSharedRef<const FUniqueNetId> >& InPlayers, bool InWasInvited)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::RegisterPlayers('%s', %d)"), *InSessionName.ToString(), InPlayers.Num());

	auto* storedSession = GetNamedSession(InSessionName);
	if (!storedSession)
	{
		UE_LOG_ONLINE(Error, TEXT("Session not found"));

		TriggerOnRegisterPlayersCompleteDelegates(InSessionName, InPlayers, false);
		return false;
	}

	if (!storedSession->SessionInfo.IsValid() || !storedSession->SessionInfo->IsValid())
	{
		UE_LOG_ONLINE(Warning, TEXT("Invalid session info"));

		TriggerOnRegisterPlayersCompleteDelegates(InSessionName, InPlayers, false);
		return false;
	}

	for (const auto& playerID : InPlayers)
	{
		if (storedSession->RegisteredPlayers.Find(playerID) != INDEX_NONE)
		{
			UE_LOG_ONLINE(Verbose, TEXT("A player already registered in a session. Skipping: playerID=%s, sessionName='%s'"), *playerID->ToDebugString(), *InSessionName.ToString());
			continue;
		}

		if (!playerID->IsValid())
			UE_LOG_ONLINE(Warning, TEXT("Invalid player ID"));

		storedSession->RegisteredPlayers.Add(playerID);

		galaxy::api::Friends()->RequestUserInformation(FUniqueNetIdGOG{*playerID});
		auto err = galaxy::api::GetError();
		if (err)
			UE_LOG_ONLINE(Warning, TEXT("Failed to request information for user: userID='%s'; '%s'; '%s'"), *playerID->ToString(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
	}

	TriggerOnRegisterPlayersCompleteDelegates(InSessionName, InPlayers, true);
	return true;
}

bool FOnlineSessionGOG::UnregisterPlayer(FName InSessionName, const FUniqueNetId& InPlayerId)
{
	return UnregisterPlayers(InSessionName, {MakeShared<FUniqueNetIdGOG>(InPlayerId)});
}

bool FOnlineSessionGOG::UnregisterPlayers(FName InSessionName, const TArray<TSharedRef<const FUniqueNetId>>& InPlayers)
{
	UE_LOG_ONLINE(Warning, TEXT("FOnlineSessionGOG::UnregisterPlayers('%s', %d)"), *InSessionName.ToString(), InPlayers.Num());

	auto* storedSession = GetNamedSession(InSessionName);
	if (!storedSession)
	{
		UE_LOG_ONLINE(Error, TEXT("Session not found"));

		TriggerOnUnregisterPlayersCompleteDelegates(InSessionName, InPlayers, false);
		return false;
	}

	if (!storedSession->SessionInfo.IsValid() || !storedSession->SessionInfo->IsValid())
	{
		UE_LOG_ONLINE(Warning, TEXT("Session info is invalid: sessionName='%s'"), *InSessionName.ToString());

		TriggerOnUnregisterPlayersCompleteDelegates(InSessionName, InPlayers, false);
		return false;
	}

	for (const auto& playerID : InPlayers)
	{
		auto playerIdx = storedSession->RegisteredPlayers.Find(playerID);
		if (playerIdx == INDEX_NONE)
		{
			UE_LOG_ONLINE(Verbose, TEXT("Player is not registered for a session. Skipping: playerID=%s, sessionName='%s'"), *playerID->ToDebugString(), *InSessionName.ToString());
			continue;
		}

		storedSession->RegisteredPlayers.RemoveAtSwap(playerIdx);
	}

	TriggerOnUnregisterPlayersCompleteDelegates(InSessionName, InPlayers, true);
	return true;
}

void FOnlineSessionGOG::RegisterLocalPlayer(const FUniqueNetId& InPlayerId, FName InSessionName, const FOnRegisterLocalPlayerCompleteDelegate& InDelegate)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::RegisterLocalPlayer('%s', '%s')"), *InSessionName.ToString(), *InPlayerId.ToString());

	InDelegate.ExecuteIfBound(InPlayerId, EOnJoinSessionCompleteResult::Success);
}

void FOnlineSessionGOG::UnregisterLocalPlayer(const FUniqueNetId& InPlayerId, FName InSessionName, const FOnUnregisterLocalPlayerCompleteDelegate& InDelegate)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::UnregisterLocalPlayer('%s', '%s')"), *InSessionName.ToString(), *InPlayerId.ToString());

	InDelegate.ExecuteIfBound(InPlayerId, true);
}

int32 FOnlineSessionGOG::GetNumSessions()
{
	UE_LOG_ONLINE(VeryVerbose, TEXT("GetNumSessions"));

	return storedSessions.Num();
}

void FOnlineSessionGOG::OnLobbyLeft(const galaxy::api::GalaxyID& InLobbyID, bool InIoFailure)
{
	UE_LOG_ONLINE(Log, TEXT("FOnlineSessionGOG::OnLobbyLeft()"));

	auto storedSession = FindSession(InLobbyID);
	if (!storedSession)
	{
		UE_LOG_ONLINE(Warning, TEXT("Lobby left listener called for an unknown session. Ignoring"));
		return;
	}

	subsystemGOG.TriggerOnConnectionStatusChangedDelegates(EOnlineServerConnectionStatus::Normal, EOnlineServerConnectionStatus::ConnectionDropped);

	// Not sure if we have to clean this up, or developer/Engine will manage everything, but let's do it
	DestroySession(storedSession->SessionName);
}

void FOnlineSessionGOG::OnGameInvitationReceived(galaxy::api::GalaxyID InUserID, const char* InConnectString)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::OnGameInvitationReceived(%s)"), UTF8_TO_TCHAR(InConnectString));

	auto sessionUrl = FUrlGOG{UTF8_TO_TCHAR(InConnectString)};

	auto sessionID = FUniqueNetIdGOG{sessionUrl.Host};
	if (!sessionID.IsValid() && !sessionID.IsLobby())
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to parse SessionID from game invitation: connectString=%s"), UTF8_TO_TCHAR(InConnectString));
		return;
	}

	auto listener = CreateListener<FGameInvitationReceivedListener>(
		*this,
		// Invites from other games are filtered by Galaxy SDK
		subsystemGOG.GetAppId(),
		*ownUserOnlineAccount->GetUserId(),
		InUserID,
		sessionID);

	galaxy::api::Matchmaking()->RequestLobbyData(sessionID, listener.Value);
	auto err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to request lobby data: lobbyID='%s'; %s: %s"),
			*sessionID.ToString(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

		FreeListener(MoveTemp(listener.Key));
		return;
	}
}

void FOnlineSessionGOG::OnGameJoinRequested(galaxy::api::GalaxyID InUserID, const char* InConnectString)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineSessionGOG::OnGameInvitationReceived(%s)"), UTF8_TO_TCHAR(InConnectString));

	auto sessionUrl = FUrlGOG{UTF8_TO_TCHAR(InConnectString)};

	auto sessionID = FUniqueNetIdGOG{sessionUrl.Host};
	if (!sessionID.IsValid() && !sessionID.IsLobby())
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to parse SessionID from game invitation: connectString=%s"), UTF8_TO_TCHAR(InConnectString));
		return;
	}

	auto listener = CreateListener<FGameInvitationAcceptedListener>(
		*this,
		// Invites from other games are filtered by Galaxy SDK
		subsystemGOG.GetAppId(),
		ownUserOnlineAccount->GetUserId(),
		InUserID,
		sessionID);

	galaxy::api::Matchmaking()->RequestLobbyData(sessionID, listener.Value);
	auto err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Warning, TEXT("Failed to request lobby data: lobbyID='%s'; %s: %s"),
			*sessionID.ToString(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

		FreeListener(MoveTemp(listener.Key));
		return;
	}
}

void FOnlineSessionGOG::DumpSessionState()
{
	UE_LOG_ONLINE(Display, TEXT("DumpSessionState"));

	for (const auto& session : storedSessions)
		DumpNamedSession(&session);
}

FNamedOnlineSession* FOnlineSessionGOG::FindSession(const FUniqueNetIdGOG& InSessionID)
{
	return storedSessions.FindByPredicate([&](const auto& session) {
		const auto& sessionInfo = session.SessionInfo;
		return sessionInfo.IsValid() && sessionInfo->IsValid() && sessionInfo->GetSessionId() == InSessionID;
	});
}

void FOnlineSessionGOG::OnLobbyDataUpdated(const galaxy::api::GalaxyID& InLobbyID, const galaxy::api::GalaxyID& InMemberID)
{
	UE_LOG_ONLINE(Display, TEXT("OnLobbyDataUpdated"));

	if (InMemberID.IsValid())
	{
		UE_LOG_ONLINE(VeryVerbose, TEXT("Received member data update. Ignored"));
		return;
	}

	FUniqueNetIdGOG sessionID{InLobbyID};

	auto storedSession = FindSession(sessionID);
	if (!storedSession)
	{
		// Session from StartSession, RequestLobbyData e.t.c will not be handed here as session are not known at this point
		UE_LOG_ONLINE(VeryVerbose, TEXT("Received unknown session data update. Ignored"));
		return;
	}

	FOnlineSessionSettings updatedSessionSettings;
	if (!OnlineSessionUtils::Fill(sessionID, updatedSessionSettings)
		|| !OnlineSessionUtils::GetSessionOpenConnections(sessionID, *storedSession))
	{
		UE_LOG_ONLINE(Error, TEXT("Error updating Session"));
		return;
	}

	// Replace previous session settings, relying on GalaxySDK to keep the data in consistency
	storedSession->SessionSettings = MoveTemp(updatedSessionSettings);
}

void FOnlineSessionGOG::OnLobbyMemberStateChanged(const galaxy::api::GalaxyID& InLobbyID, const galaxy::api::GalaxyID& /*InMemberID*/, galaxy::api::LobbyMemberStateChange InMemberStateChange)
{
	UE_LOG_ONLINE(Display, TEXT("OnLobbyMemberStateChanged"));

	auto storedSession = FindSession(InLobbyID);
	if (!storedSession)
	{
		UE_LOG_ONLINE(VeryVerbose, TEXT("Received unknown session member state update. Ignored"));
		return;
	}

	if (InMemberStateChange == galaxy::api::LOBBY_MEMBER_STATE_CHANGED_ENTERED)
		--storedSession->NumOpenPublicConnections;
	else
		++storedSession->NumOpenPublicConnections;
}
