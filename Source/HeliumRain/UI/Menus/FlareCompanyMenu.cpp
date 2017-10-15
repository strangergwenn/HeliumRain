
#include "FlareCompanyMenu.h"
#include "../../Flare.h"

#include "../Components/FlarePartInfo.h"
#include "../Components/FlareCompanyInfo.h"

#include "../../Data/FlareSpacecraftComponentsCatalog.h"
#include "../../Data/FlareCustomizationCatalog.h"

#include "../../Game/FlareGame.h"
#include "../../Game/FlareCompany.h"

#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareCompanyMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareCompanyMenu::Construct(const FArguments& InArgs)
{
	// Data
	Company = NULL;
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	AFlarePlayerController* PC = MenuManager->GetPC();
	
	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SHorizontalBox)

		// Content block
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.AutoWidth()
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			.Padding(Theme.TitlePadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.SubTitleFont)
				.Text(LOCTEXT("CompanyInfoTitle", "Company"))
			]

			// Company info
			+ SScrollBox::Slot()
			.Padding(Theme.ContentPadding)
			[
				SNew(SBox)
				.WidthOverride(0.8 * Theme.ContentWidth)
				[
					SAssignNew(CompanyInfo, SFlareCompanyInfo)
					.Player(PC)
				]
			]

			// TR info
			+ SScrollBox::Slot()
			[
				SAssignNew(TradeRouteInfo, SFlareTradeRouteInfo)
				.MenuManager(MenuManager)
			]

			// Property list
			+ SScrollBox::Slot()
			[
				SNew(SBox)
				.WidthOverride(Theme.ContentWidth)
				.HAlign(HAlign_Left)
				[
					SAssignNew(ShipList, SFlareList)
					.MenuManager(MenuManager)
					.Title(LOCTEXT("Property", "Property"))
				]
			]
		]

		// Company customization
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.Padding(Theme.TitlePadding)
			.AutoHeight()
			[
				SNew(STextBlock)
				.TextStyle(&Theme.SubTitleFont)
				.Text(LOCTEXT("CompanyEditTitle", "Company appearance"))
			]

			// Company name
			+ SVerticalBox::Slot()
			.Padding(Theme.ContentPadding)
			.AutoHeight()
			.HAlign(HAlign_Fill)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(LOCTEXT("EditCompanyName", "Company name"))
				]

				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				[
					SNew(SBox)
					.WidthOverride(0.4 * Theme.ContentWidth)
					[
						SNew(SBorder)
						.BorderImage(&Theme.BackgroundBrush)
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(CompanyName, SEditableText)
							.AllowContextMenu(false)
							.Style(&Theme.TextInputStyle)
						]
					]
				]
			
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				[
					SNew(SFlareButton)
					.Text(LOCTEXT("Rename", "Rename"))
					.HelpText(LOCTEXT("RenameInfo", "Rename this company"))
					.Icon(FFlareStyleSet::GetIcon("OK"))
					.OnClicked(this, &SFlareCompanyMenu::OnRename)
					.IsDisabled(this, &SFlareCompanyMenu::IsRenameDisabled)
					.Width(4)
				]
			]

			// Color picker
			+ SVerticalBox::Slot()
			.Padding(Theme.ContentPadding)
			.AutoHeight()
			[
				SAssignNew(ColorBox, SFlareColorPanel)
				.MenuManager(MenuManager)
			]
				
			// Emblem
			+ SVerticalBox::Slot()
			.Padding(Theme.ContentPadding)
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Top)
			[
				SAssignNew(EmblemPicker, SFlareDropList<int32>)
				.LineSize(1)
				.HeaderWidth(3)
				.HeaderHeight(3)
				.ItemWidth(3)
				.ItemHeight(2.7)
				.ShowColorWheel(false)
				.MaximumHeight(600)
				.OnItemPicked(this, &SFlareCompanyMenu::OnEmblemPicked)
			]
		]
	];
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

bool SFlareCompanyMenu::IsRenameDisabled() const
{
	FString CompanyNameData = CompanyName->GetText().ToString();

	if (CompanyNameData.Len() > 25)
	{
		return true;
	}
	if (CompanyNameData == Company->GetCompanyName().ToString())
	{
		return true;
	}
	else
	{
		return false;
	}
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareCompanyMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareCompanyMenu::Enter(UFlareCompany* Target)
{
	FLOG("SFlareCompanyMenu::Enter");
	SetEnabled(true);

	// Company data
	Company = Target;
	SetVisibility(EVisibility::Visible);
	CompanyInfo->SetCompany(Company);
	CompanyName->SetText(Company->GetCompanyName());
	TradeRouteInfo->UpdateTradeRouteList();

	// Player specific
	AFlarePlayerController* PC = MenuManager->GetPC();
	if (PC && Target)
	{
		// Colors
		FFlarePlayerSave Data;
		FFlareCompanyDescription Unused;
		PC->Save(Data, Unused);
		ColorBox->Setup(Data);

		// Emblems
		UFlareCustomizationCatalog* CustomizationCatalog = MenuManager->GetGame()->GetCustomizationCatalog();
		for (int i = 0; i < CustomizationCatalog->GetEmblemCount(); i++)
		{
			EmblemPicker->AddItem(SNew(SImage).Image(CustomizationCatalog->GetEmblemBrush(i)));
		}
		EmblemPicker->SetSelectedIndex(PC->GetPlayerData()->PlayerEmblemIndex);

		// Menu
		PC->GetMenuPawn()->SetCameraOffset(FVector2D(100, -30));
		if (PC->GetPlayerShip())
		{
			PC->GetMenuPawn()->ShowShip(PC->GetPlayerShip());
		}
		else
		{
			const FFlareSpacecraftComponentDescription* PartDesc = PC->GetGame()->GetShipPartsCatalog()->Get("object-safe");
			PC->GetMenuPawn()->ShowPart(PartDesc);
		}
		
		// Station list
		TArray<UFlareSimulatedSpacecraft*>& CompanyStations = Target->GetCompanyStations();
		for (int32 i = 0; i < CompanyStations.Num(); i++)
		{
			if (CompanyStations[i]->GetDamageSystem()->IsAlive())
			{
				ShipList->AddShip(CompanyStations[i]);
			}
		}

		// Ship list
		TArray<UFlareSimulatedSpacecraft*>& CompanyShips = Target->GetCompanyShips();
		for (int32 i = 0; i < CompanyShips.Num(); i++)
		{
			if (CompanyShips[i]->GetDamageSystem()->IsAlive())
			{
				ShipList->AddShip(CompanyShips[i]);
			}
		}
	}

	ShipList->RefreshList();
	ShipList->SetVisibility(EVisibility::Visible);
}

void SFlareCompanyMenu::Exit()
{
	SetEnabled(false);
	ShipList->Reset();
	ShipList->SetVisibility(EVisibility::Collapsed);

	EmblemPicker->ClearItems();
	TradeRouteInfo->Clear();

	Company = NULL;
	SetVisibility(EVisibility::Collapsed);
}

void SFlareCompanyMenu::OnRename()
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	FFlareCompanyDescription CompanyDescription = *PC->GetCompanyDescription();
	CompanyDescription.Name = CompanyName->GetText();
	PC->SetCompanyDescription(CompanyDescription);
}

void SFlareCompanyMenu::OnEmblemPicked(int32 Index)
{
	AFlarePlayerController* PC = MenuManager->GetPC();
	UFlareCustomizationCatalog* CustomizationCatalog = MenuManager->GetGame()->GetCustomizationCatalog();

	FFlareCompanyDescription CompanyDescription = *PC->GetCompanyDescription();
	CompanyDescription.Emblem = CustomizationCatalog->GetEmblem(Index);
	PC->SetCompanyDescription(CompanyDescription);
	PC->GetPlayerData()->PlayerEmblemIndex = EmblemPicker->GetSelectedIndex();
	PC->OnLoadComplete();
}


#undef LOCTEXT_NAMESPACE
