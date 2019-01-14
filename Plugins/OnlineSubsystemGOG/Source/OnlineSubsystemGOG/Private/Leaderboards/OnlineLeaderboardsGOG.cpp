#include "OnlineLeaderboardsGOG.h"

#include "OnlineSubsystemGOG.h"
#include "ReadLeaderboardForUsersListener.h"
#include "ReadLeaderboardAroundRankListener.h"
#include "ReadLeaderboardAroundUserListener.h"
#include "FlushLeaderboardsListener.h"
#include "Converters/OnlineLeaderboardConverter.h"
#include "Types/UniqueNetIdGOG.h"
#include "VariantDataUtils.h"
#include "Friends/OnlineFriendsGOG.h"

#include "OnlineSubsystemUtils.h"
#include "Types/UserOnlineAccountGOG.h"

namespace
{

	galaxy::api::LeaderboardSortMethod ConverterLeaderboardSortMethod(ELeaderboardSort::Type InSortMethod)
	{
		switch (InSortMethod)
		{
			case ELeaderboardSort::None:
				return galaxy::api::LEADERBOARD_SORT_METHOD_NONE;
			case ELeaderboardSort::Ascending:
				return galaxy::api::LEADERBOARD_SORT_METHOD_ASCENDING;
			case ELeaderboardSort::Descending:
				return galaxy::api::LEADERBOARD_SORT_METHOD_DESCENDING;
		}

		checkf(false, TEXT("Invalid leaderboard sort method: %u"), InSortMethod);
		return galaxy::api::LEADERBOARD_SORT_METHOD_NONE;
	}

	galaxy::api::LeaderboardDisplayType ConverterLeaderboardDisplayFormat(ELeaderboardFormat::Type InDisplayFormat)
	{
		switch (InDisplayFormat)
		{
			case ELeaderboardFormat::Number:
				return galaxy::api::LEADERBOARD_DISPLAY_TYPE_NUMBER;
			case ELeaderboardFormat::Seconds:
				return galaxy::api::LEADERBOARD_DISPLAY_TYPE_TIME_SECONDS;
			case ELeaderboardFormat::Milliseconds:
				return galaxy::api::LEADERBOARD_DISPLAY_TYPE_TIME_MILLISECONDS;
		}

		checkf(false, TEXT("Invalid leaderboard display type: %u"), InDisplayFormat);
		return galaxy::api::LEADERBOARD_DISPLAY_TYPE_NONE;
	}

	bool GetMainLeaderboardScore(FOnlineLeaderboardWrite& InWriteLeaderboard, int32& OutMainScore)
	{
		auto ratedStat = InWriteLeaderboard.FindStatByName(InWriteLeaderboard.RatedStat);
		if (!ratedStat)
		{
			UE_LOG_ONLINE(Error, TEXT("Rated stat not found in leaderboard data"));
			return false;
		}

		return SafeGetInt32Value(*ratedStat, OutMainScore);
	}

	auto GetStatsForLeaderboardDetails(const FName& InRatedStatName, FStatPropertyArray InLeaderboardStats)
	{
		// Copy all rows except main stat
		InLeaderboardStats.Remove(InRatedStatName);
		return InLeaderboardStats;
	}

	FString GetSerializedDetails(FOnlineLeaderboardWrite& InWriteLeaderboard)
	{
		auto serializedData = OnlineLeaderboardConverter::ToJsonString(GetStatsForLeaderboardDetails(InWriteLeaderboard.RatedStat, InWriteLeaderboard.Properties));

		constexpr uint32_t MAX_LEADERBOARD_DETAIL_SIZE = 3071;

		if (CharLen(serializedData) > MAX_LEADERBOARD_DETAIL_SIZE)
		{
			UE_LOG_ONLINE(Error, TEXT("Serialized data for leaderboard is to long. Please report this to the GalaxySDK team: datailsSize=%d"), CharLen(serializedData));
			check(false && "Serialized data for leaderboard is to long. Please report this to the GalaxySDK team");
		}

		return serializedData;
	}

	bool ShouldUpdateLeaderboardCached(ELeaderboardUpdateMethod::Type InUpdateMethod, ELeaderboardSort::Type InSortMethod, int32 InOldScore, int32 InNewScore)
	{
		return InUpdateMethod == ELeaderboardUpdateMethod::Force
			|| (InSortMethod == ELeaderboardSort::Descending && InOldScore < InNewScore)
			|| (InSortMethod == ELeaderboardSort::Ascending && InOldScore > InNewScore);
	}

}

FOnlineLeaderboardsGOG::FOnlineLeaderboardsGOG(const FOnlineSubsystemGOG& InOnlineSubsystemGOG, TSharedRef<FUserOnlineAccountGOG> InUserOnlineAccount)
	: onlineSubsystemGOG{InOnlineSubsystemGOG}
	, ownUserOnlineAccount{MoveTemp(InUserOnlineAccount)}
{
}

bool FOnlineLeaderboardsGOG::MarkLeaderboardStarted(FOnlineLeaderboardReadRef& InOutReadLeaderboard, IOnlineLeaderboards* onlineLeaderboardsInterface) const
{
	if (InOutReadLeaderboard->ReadState == EOnlineAsyncTaskState::InProgress)
	{
		UE_LOG_ONLINE(Warning, TEXT("There seems to be another ReadLeaderboard() call made: leaderboardName='%s'"), *InOutReadLeaderboard->LeaderboardName.ToString());
		// Everything else will be done by appropriate call
		return false;
	}

	if (InOutReadLeaderboard->LeaderboardName.IsNone())
	{
		UE_LOG_ONLINE(Error, TEXT("Empty leaderboard name"));
		InOutReadLeaderboard->ReadState = EOnlineAsyncTaskState::Failed;
		onlineLeaderboardsInterface->TriggerOnLeaderboardReadCompleteDelegates(false);
		return false;
	}

	InOutReadLeaderboard->ReadState = EOnlineAsyncTaskState::InProgress;
	return true;
}

bool FOnlineLeaderboardsGOG::ReadLeaderboards(const TArray<TSharedRef<const FUniqueNetId>>& InPlayers, FOnlineLeaderboardReadRef& InOutReadLeaderboard)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineLeaderboardsGOG::ReadLeaderboards()"));

	if (!InPlayers.Num())
	{
		UE_LOG_ONLINE(Error, TEXT("Empty users array"));
		InOutReadLeaderboard->ReadState = EOnlineAsyncTaskState::Failed;
		TriggerOnLeaderboardReadCompleteDelegates(false);
		return false;
	}

	if (!MarkLeaderboardStarted(InOutReadLeaderboard, this))
		return false;

	auto listener = CreateListener<FReadLeaderboardForUsersListener>(*this, InPlayers, InOutReadLeaderboard);

	galaxy::api::Stats()->FindLeaderboard(
		TCHAR_TO_UTF8(*InOutReadLeaderboard->LeaderboardName.ToString()),
		listener.Value);

	auto err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Error, TEXT("Failed to request leaderboard definitions: leaderboardName='%s'; %s; %s"),
			*InOutReadLeaderboard->LeaderboardName.ToString(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
		InOutReadLeaderboard->ReadState = EOnlineAsyncTaskState::Failed;

		FreeListener(MoveTemp(listener.Key));

		TriggerOnLeaderboardReadCompleteDelegates(false);
		return false;
	}

	return true;
}

bool FOnlineLeaderboardsGOG::ReadLeaderboardsForFriends(int32 InLocalUserNum, FOnlineLeaderboardReadRef& InOutReadLeaderboard)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineLeaderboardsGOG::ReadLeaderboardsForFriends()"));

	CheckLocalUserNum(InLocalUserNum);

	auto onlineFriendsInterface = StaticCastSharedPtr<FOnlineFriendsGOG>(onlineSubsystemGOG.GetFriendsInterface());
	if (!onlineFriendsInterface.IsValid())
	{
		UE_LOG_ONLINE(Error, TEXT("Invalid OnlineFriends interface"));

		InOutReadLeaderboard->ReadState = EOnlineAsyncTaskState::Failed;
		TriggerOnLeaderboardReadCompleteDelegates(false);
		return false;
	}

	TArray<TSharedRef<FOnlineFriend>> friendList;
	if (!onlineFriendsInterface->GetFriendsList(InLocalUserNum, onlineFriendsInterface->GetDefaultFriendsListName(), friendList))
	{
		InOutReadLeaderboard->ReadState = EOnlineAsyncTaskState::Failed;
		TriggerOnLeaderboardReadCompleteDelegates(false);
		return false;
	}

	TArray<TSharedRef<const FUniqueNetId>> friendIDlist;
	friendIDlist.Reserve(friendIDlist.Num());

	for (const auto& _friend : friendList)
		friendIDlist.Emplace(_friend->GetUserId());

	friendIDlist.Emplace(ownUserOnlineAccount->GetUserId());

	return ReadLeaderboards(friendIDlist, InOutReadLeaderboard);
}

bool FOnlineLeaderboardsGOG::ReadLeaderboardsAroundRank(int32 InRank, uint32 InRange, FOnlineLeaderboardReadRef& InOutReadLeaderboard)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineLeaderboardsGOG::ReadLeaderboardsAroundRank()"));

	if (InRank < 0)
	{
		UE_LOG_ONLINE(Error, TEXT("Invalid rank or range: rank=%d"), InRank);

		InOutReadLeaderboard->ReadState = EOnlineAsyncTaskState::Failed;
		TriggerOnLeaderboardReadCompleteDelegates(false);
		return false;
	}

	if (!MarkLeaderboardStarted(InOutReadLeaderboard, this))
		return false;

	auto listener = CreateListener<FReadLeaderboardAroundRankListener>(*this, InRank, InRange, InOutReadLeaderboard);
	galaxy::api::Stats()->FindLeaderboard(
		TCHAR_TO_UTF8(*InOutReadLeaderboard->LeaderboardName.ToString()),
		listener.Value);

	auto err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Error, TEXT("Failed to request leaderboard definitions: leaderboardName='%s'; %s; %s"),
			*InOutReadLeaderboard->LeaderboardName.ToString(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
		InOutReadLeaderboard->ReadState = EOnlineAsyncTaskState::Failed;

		FreeListener(MoveTemp(listener.Key));

		TriggerOnLeaderboardReadCompleteDelegates(false);
		return false;
	}

	return true;
}

bool FOnlineLeaderboardsGOG::ReadLeaderboardsAroundUser(TSharedRef<const FUniqueNetId> InPlayer, uint32 InRange, FOnlineLeaderboardReadRef& InOutReadLeaderboard)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineLeaderboardsGOG::ReadLeaderboardsAroundUser()"));

	if (!InPlayer->IsValid())
	{
		UE_LOG_ONLINE(Error, TEXT("Invalid Player ID: playerID='%s'"), *InPlayer->ToString());

		InOutReadLeaderboard->ReadState = EOnlineAsyncTaskState::Failed;
		TriggerOnLeaderboardReadCompleteDelegates(false);
		return false;
	}

	if (!MarkLeaderboardStarted(InOutReadLeaderboard, this))
		return false;

	auto listener = CreateListener<FReadLeaderboardAroundUserListener>(*this, StaticCastSharedRef<const FUniqueNetIdGOG>(InPlayer), InRange, InOutReadLeaderboard);
	galaxy::api::Stats()->FindLeaderboard(
		TCHAR_TO_UTF8(*InOutReadLeaderboard->LeaderboardName.ToString()),
		listener.Value);
	auto err = galaxy::api::GetError();
	if (err)
	{
		UE_LOG_ONLINE(Error, TEXT("Failed to request leaderboard definitions: leaderboardName='%s'; %s; %s"),
			*InOutReadLeaderboard->LeaderboardName.ToString(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));
		InOutReadLeaderboard->ReadState = EOnlineAsyncTaskState::Failed;

		FreeListener(MoveTemp(listener.Key));

		TriggerOnLeaderboardReadCompleteDelegates(false);
		return false;
	}

	return true;
}

void FOnlineLeaderboardsGOG::FreeStats(FOnlineLeaderboardRead& InOutReadLeaderboard)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineLeaderboardsGOG::ReadLeaderboardsAroundUser"));
	// Nothing to do
}

bool FOnlineLeaderboardsGOG::WriteLeaderboards(const FName& InSessionName, const FUniqueNetId& InPlayer, FOnlineLeaderboardWrite& InWriteLeaderboard)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineLeaderboardsGOG::WriteLeaderboards()"));

	if (*ownUserOnlineAccount->GetUserId() != InPlayer)
	{
		UE_LOG_ONLINE(Error, TEXT("Invalid Player ID"));
		return false;
	}

	if (!InWriteLeaderboard.LeaderboardNames.Num())
	{
		UE_LOG_ONLINE(Error, TEXT("No leaderboard names provided"));
		return false;
	}

	if (!InWriteLeaderboard.Properties.Num())
	{
		UE_LOG_ONLINE(Display, TEXT("No stats to be written"));
		return false;
	}

	return UpdateWriteCache(InSessionName, InWriteLeaderboard);
}

bool FOnlineLeaderboardsGOG::UpdateWriteCache(const FName& InSessionName, FOnlineLeaderboardWrite &InWriteLeaderboard)
{
	int32 newScore;
	if (!GetMainLeaderboardScore(InWriteLeaderboard, newScore))
	{
		UE_LOG_ONLINE(Error, TEXT("Rated stat must be an integer value in Int32 range"));
		return false;
	}

	auto details = GetSerializedDetails(InWriteLeaderboard);

	auto& sessionCache = writeLeaderboardCache.FindOrAdd(InSessionName);
	for (const auto& leaderboardToAdd : InWriteLeaderboard.LeaderboardNames)
	{
		auto cachedLeadeboard = sessionCache.Find(leaderboardToAdd);
		if (!cachedLeadeboard)
		{
			sessionCache.Emplace(
				leaderboardToAdd,
				CachedLeaderboardDetails{newScore, details, InWriteLeaderboard.UpdateMethod, InWriteLeaderboard.SortMethod, InWriteLeaderboard.DisplayFormat});
			continue;
		}

		if (ShouldUpdateLeaderboardCached(InWriteLeaderboard.UpdateMethod, InWriteLeaderboard.SortMethod, cachedLeadeboard->MainScore, newScore))
		{
			cachedLeadeboard->MainScore = newScore;
			cachedLeadeboard->SerializedDetails = details;
			cachedLeadeboard->UpdateMethod = InWriteLeaderboard.UpdateMethod;
			cachedLeadeboard->SortMethod = InWriteLeaderboard.SortMethod;
			cachedLeadeboard->DisplayFormat = InWriteLeaderboard.DisplayFormat;
		}
	}

	return true;
}

bool FOnlineLeaderboardsGOG::FlushLeaderboards(const FName& InSessionName)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineLeaderboardsGOG::FlushLeaderboards()"), *InSessionName.ToString());

	auto cachedLeaderboards = writeLeaderboardCache.Find(InSessionName);
	if (!cachedLeaderboards)
	{
		UE_LOG_ONLINE(Display, TEXT("Leaderboard not found or already flushed"), *InSessionName.ToString());
		TriggerOnLeaderboardFlushCompleteDelegates(InSessionName, false);
		return false;
	}

	auto listener = CreateListener<FFlushLeaderboardsListener>(*this, InSessionName, *cachedLeaderboards);

	for (const auto& cachedLeaderboard : *cachedLeaderboards)
	{
		galaxy::api::Stats()->FindOrCreateLeaderboard(
			TCHAR_TO_UTF8(*cachedLeaderboard.Key.ToString()),
			TCHAR_TO_UTF8(*cachedLeaderboard.Key.ToString()),
			ConverterLeaderboardSortMethod(cachedLeaderboard.Value.SortMethod),
			ConverterLeaderboardDisplayFormat(cachedLeaderboard.Value.DisplayFormat),
			listener.Value);

		auto err = galaxy::api::GetError();
		if (err)
		{
			UE_LOG_ONLINE(Error, TEXT("Failed to find or create leaderboard: sessionName='%s', leaderboardName='%s', %s; %s"),
				*InSessionName.ToString(), *cachedLeaderboard.Key.ToString(), UTF8_TO_TCHAR(err->GetName()), UTF8_TO_TCHAR(err->GetMsg()));

			FreeListener(MoveTemp(listener.Key));

			TriggerOnLeaderboardFlushCompleteDelegates(InSessionName, false);
			return false;
		}
	}

	return true;
}

bool FOnlineLeaderboardsGOG::WriteOnlinePlayerRatings(const FName&, int32, const TArray<FOnlinePlayerScore>&)
{
	UE_LOG_ONLINE(Display, TEXT("FOnlineLeaderboardsGOG::ReadLeaderboardsAroundUser"));
	// TODO: FOnlinePlayerScore is not implemented in UE4
	return false;
}

void FOnlineLeaderboardsGOG::RemoveCachedLeaderboard(const FName InSessionName)
{
	writeLeaderboardCache.Remove(InSessionName);
}
