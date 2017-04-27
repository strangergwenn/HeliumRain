#pragma once

#include "../Spacecrafts/FlareSpacecraftTypes.h"
#include "FlareUITypes.generated.h"


/** Possible menu targets */
UENUM()
namespace EFlareMenu
{
	enum Type
	{
		MENU_None,

		// Boot menus
		MENU_Main,
		MENU_NewGame,

		// Special "menus" for async transitions
		MENU_CreateGame,
		MENU_LoadGame,
		MENU_FlyShip,
		MENU_Travel,
		MENU_ReloadSector,
		MENU_FastForwardSingle,
		MENU_GameOver,

		// Main gameplay menus
		MENU_Story,
		MENU_Company,
		MENU_Fleet,
		MENU_Quest,
		MENU_Ship,
		MENU_ShipConfig,
		MENU_Station,
		MENU_Sector,
		MENU_Trade,
		MENU_TradeRoute,
		MENU_Orbit,
		MENU_Leaderboard,
		MENU_ResourcePrices,
		MENU_WorldEconomy,
		MENU_Technology,

		// Support menus
		MENU_Settings,
		MENU_Credits,
		MENU_EULA,
		MENU_Quit
	};
}


/** Menu parameter structure storing commands + data for async processing */
struct FFlareMenuParameterData
{
	FFlareMenuParameterData()
		: Company(NULL)
		, Factory(NULL)
		, Fleet(NULL)
		, Quest(NULL)
		, Route(NULL)
		, Sector(NULL)
		, Spacecraft(NULL)
		, Travel(NULL)
		, Resource(NULL)
		, ScenarioIndex(0)
		, PlayerEmblemIndex(0)
	{}

	class UFlareCompany*                        Company;
	class UFlareFactory*                        Factory;
	class UFlareFleet*                          Fleet;
	class UFlareQuest*                          Quest;
	class UFlareTradeRoute*                     Route;
	class UFlareSimulatedSector*                Sector;
	class UFlareSimulatedSpacecraft*            Spacecraft;
	class UFlareTravel*                         Travel;
	struct FFlareResourceDescription*           Resource;

	struct FFlareCompanyDescription*            CompanyDescription;
	int32                                       ScenarioIndex;
	int32                                       PlayerEmblemIndex;
	bool                                        PlayTutorial;
};


/** Possible notification types */
UENUM()
namespace EFlareNotification
{
	enum Type
	{
		NT_Info,
		NT_Economy,
		NT_Military,
		NT_Quest
	};
}


// Delegates
DECLARE_DELEGATE_OneParam(FOrderDelegate, FFlareSpacecraftDescription*)



UCLASS()
class HELIUMRAIN_API UFlareUITypes : public UObject
{
	GENERATED_UCLASS_BODY()

};
