#pragma once

#include "CommonGOG.h"
#include "Types/IListenerGOG.h"
#include "Types/UniqueNetIdGOG.h"
#include "ListenerManager.h"
#include "Types/UserOnlineAccountGOG.h"

#include "Interfaces/OnlineAchievementsInterface.h"

class FOnlineAchievementsGOG
	: public IOnlineAchievements
	, public FListenerManager
	, public galaxy::api::GlobalAchievementChangeListener
#if !UE_BUILD_SHIPPING
	, public galaxy::api::IStatsAndAchievementsStoreListener
#endif
{
public:

	void WriteAchievements(const FUniqueNetId& InPlayerId, FOnlineAchievementsWriteRef& InWriteObject, const FOnAchievementsWrittenDelegate& InDelegate) override;

	void QueryAchievements(const FUniqueNetId& InPlayerId, const FOnQueryAchievementsCompleteDelegate& InDelegate) override;

	void QueryAchievementDescriptions(const FUniqueNetId& InPlayerId, const FOnQueryAchievementsCompleteDelegate& InDelegate) override;

	EOnlineCachedResult::Type GetCachedAchievement(const FUniqueNetId& InPlayerId, const FString& InAchievementId, FOnlineAchievement& OutAchievement) override;

	EOnlineCachedResult::Type GetCachedAchievements(const FUniqueNetId& InPlayerId, TArray<FOnlineAchievement>& OutAchievements) override;

	EOnlineCachedResult::Type GetCachedAchievementDescription(const FString& InAchievementId, FOnlineAchievementDesc& OutAchievementDesc) override;

#if !UE_BUILD_SHIPPING
	/**
	 * Resets achievements for a given player
	 *
	 * Reset achievements in both OnlineSubsystem cache and GOG Platforms.
	 * This operation waits until operation finishes on GOG servers, so might be slow.
	 *
	 * @param InPlayerId - The uid of the player
	 * @return true if achievements are cleared
	*/
	bool ResetAchievements(const FUniqueNetId& InPlayerId) override;
#endif

	~FOnlineAchievementsGOG() = default;

PACKAGE_SCOPE:

	FOnlineAchievementsGOG() = delete;
	FOnlineAchievementsGOG(class FOnlineSubsystemGOG& InSubsystem, TSharedRef<FUserOnlineAccountGOG> InUserOnlineAccount);

	void OnAchievementsRetrieved(const FUniqueNetIdGOG& InPlayerID);

private:

	inline auto AchivementsCount() const;

	inline bool AssertAchievementsCount() const;

	inline bool AreAchivementsDescriptionsAvailable() const;

	void OnAchievementUnlocked(const char* InName) override;

	void AddOrReplacePlayerAchievements(const FUniqueNetIdGOG& InPlayerID);

	bool UpdateAchievementDescriptions();

	TArray<FString> achievementIDs;
	TMap<FString, FOnlineAchievementDesc> cachedAchievementDescriptions;
	TMap<FUniqueNetIdGOG, TArray<FOnlineAchievement>> cachedAchievements;

#if !UE_BUILD_SHIPPING
	void OnUserStatsAndAchievementsStoreSuccess() override;
	void OnUserStatsAndAchievementsStoreFailure(galaxy::api::IStatsAndAchievementsStoreListener::FailureReason failureReason) override;

	bool areAchivementsReset{false};
	bool achivementsResetResult{false};
#endif

	FOnlineSubsystemGOG& subsystemGOG;
	TSharedRef<FUserOnlineAccountGOG> ownUserOnlineAccount;
};
