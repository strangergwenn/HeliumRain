#pragma once

#include "../Spacecrafts/FlareSpacecraftTypes.h"

#include <SlateBasics.h>
#include <Slate/SlateGameResources.h>
#include "../UI/Style/FlareStyleSet.h"

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

		// Skirmish mode
		MENU_SkirmishSetup,
		MENU_SkirmishScore,

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
		, Skirmish(NULL)
		, Resource(NULL)
		, CompanyDescription(NULL)
		, ScenarioIndex(0)
		, PlayerEmblemIndex(0)
		, PlayTutorial(false)
		, OrderForPlayer(false)
		, SpacecraftOrderHeavy(false)
		, ComplexConnectorName(NAME_None)
	{}

	class UFlareCompany*                        Company;
	class UFlareFactory*                        Factory;
	class UFlareFleet*                          Fleet;
	class UFlareQuest*                          Quest;
	class UFlareTradeRoute*                     Route;
	class UFlareSimulatedSector*                Sector;
	class UFlareSimulatedSpacecraft*            Spacecraft;
	class UFlareTravel*                         Travel;
	class UFlareSkirmishManager*               Skirmish;
	struct FFlareResourceDescription*           Resource;
	struct FFlareCompanyDescription*            CompanyDescription;

	int32                                       ScenarioIndex;
	int32                                       PlayerEmblemIndex;
	bool                                        PlayTutorial;
	bool                                        OrderForPlayer;
	bool                                        SpacecraftOrderHeavy;
	FName                                       ComplexConnectorName;
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
		NT_MilitarySilent,
		NT_NewQuest,
		NT_Quest
	};
}

/** Possible sort types in economy menus */
UENUM()
namespace EFlareEconomySort
{
	enum Type
	{
		ES_Sector,
		ES_Resource,
		ES_Production,
		ES_Consumption,
		ES_Stock,
		ES_Needs,
		ES_Price,
		ES_Variation,
		ES_Transport
	};
}


// Economy table width sizes
#define ECONOMY_TABLE_WIDTH_SMALL    0.2f
#define ECONOMY_TABLE_BUTTON_SMALL   3.65f
#define ECONOMY_TABLE_WIDTH_MEDIUM   0.3f
#define ECONOMY_TABLE_BUTTON_MEDIUM  4.0f
#define ECONOMY_TABLE_WIDTH_LARGE    0.40f
#define ECONOMY_TABLE_BUTTON_LARGE   6.5f
#define ECONOMY_TABLE_WIDTH_FULL     2.1f


// Delegates
DECLARE_DELEGATE_OneParam(FOrderDelegate, FFlareSpacecraftDescription*)


UCLASS()
class HELIUMRAIN_API UFlareUITypes : public UObject
{
public:

	GENERATED_UCLASS_BODY()


	/** Add a header */
	static TSharedRef<SWidget> Header(FText Title);

};
