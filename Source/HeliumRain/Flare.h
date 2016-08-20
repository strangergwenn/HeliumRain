
#ifndef __FLARE_H__
#define __FLARE_H__

#include "Engine.h"
#include "UI/Style/FlareStyleSet.h"


/*----------------------------------------------------
	UI enums
----------------------------------------------------*/

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
		MENU_LoadGame,
		MENU_FlyShip,
		MENU_Travel,
		MENU_ReloadSector,

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

		// Support menus
		MENU_Settings,
		MENU_Credits,
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
};


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

#define NULL_QUAT           (FQuat::MakeFromEuler(FVector::ZeroVector))

#define FLOG(Format)        UE_LOG(LogTemp, Display, TEXT(Format))
#define FLOGV(Format, ...)  UE_LOG(LogTemp, Display, TEXT(Format), __VA_ARGS__)

DECLARE_LOG_CATEGORY_EXTERN(LogFlare, Log, All);


/*----------------------------------------------------
	Game module definition
----------------------------------------------------*/

class FFlareModule : public FDefaultGameModuleImpl
{

public:

	void StartupModule() override;

	void ShutdownModule() override;


protected:

	FFlareStyleSet StyleInstance;

};


#endif
