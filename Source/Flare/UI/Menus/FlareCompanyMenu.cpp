
#include "../../Flare.h"
#include "FlareStationMenu.h"
#include "../Widgets/FlarePartInfo.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareHUD.h"
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
	OwnerHUD = InArgs._OwnerHUD;
	TSharedPtr<SFlareButton> BackButton;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	const FFlareContainerStyle* DefaultContainerStyle = &FFlareStyleSet::Get().GetWidgetStyle<FFlareContainerStyle>("/Style/DefaultContainerStyle");
	
	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SHorizontalBox)

		// UI container
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.AutoWidth()
		[
			SNew(SBorder)
			.BorderImage(&DefaultContainerStyle->BackgroundBrush)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(SVerticalBox)

					// Object name
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(10))
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SImage).Image(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Company))
						]

						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Title", "COMPANY"))
							.TextStyle(FFlareStyleSet::Get(), "Flare.Title1")
						]
					]

					// Title
					+ SVerticalBox::Slot()
					.Padding(FMargin(10))
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Overview", "OVERVIEW"))
						.TextStyle(FFlareStyleSet::Get(), "Flare.Title2")
					]

					// Company info
					+ SVerticalBox::Slot()
					.Padding(FMargin(10))
					.AutoHeight()
					[
						SAssignNew(ActionMenu, SFlareTargetActions)
						.Player(PC)
						.MinimizedMode(true)
					]

					// Color picker
					+ SVerticalBox::Slot()
					.Padding(FMargin(10))
					.AutoHeight()
					[
						SAssignNew(ColorBox, SFlareColorPanel).OwnerHUD(OwnerHUD)
					]

					// Object list
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(ShipList, SFlareShipList)
						.OwnerHUD(OwnerHUD)
						.Title(LOCTEXT("Property", "PROPERTY"))
					]
				]
			]
		]

		// Dashboard button
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		[
			SAssignNew(BackButton, SFlareButton)
			.ContainerStyle(FFlareStyleSet::Get(), "/Style/InvisibleContainerStyle")
			.ButtonStyle(FFlareStyleSet::Get(), "/Style/BackToDashboardButton")
			.OnClicked(this, &SFlareCompanyMenu::OnDashboardClicked)
		]
	];

	// Dashboard button
	BackButton->GetContainer()->SetContent(SNew(SImage).Image(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Exit)));
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareCompanyMenu::Setup(FFlarePlayerSave& PlayerData)
{
	SetEnabled(false);
	ColorBox->Setup(PlayerData);
	SetVisibility(EVisibility::Hidden);	
}

void SFlareCompanyMenu::Enter(UFlareCompany* Target)
{
	FLOG("SFlareCompanyMenu::Enter");
	SetEnabled(true);

	// Company data
	Company = Target;
	ActionMenu->SetCompany(Target);
	ActionMenu->Show();
	SetVisibility(EVisibility::Visible);

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC && Target)
	{
		// Menu
		const FFlareShipComponentDescription* PartDesc = PC->GetGame()->GetShipPartsCatalog()->Get("object-safe");
		PC->GetMenuPawn()->SetHorizontalOffset(100);
		PC->GetMenuPawn()->UpdateBackgroundColor(0.1, 0.8);
		PC->GetMenuPawn()->ShowPart(PartDesc);
		
		// Station list
		TArray<IFlareStationInterface*>& CompanyStations = Target->GetCompanyStations();
		for (int32 i = 0; i < CompanyStations.Num(); i++)
		{
			ShipList->AddStation(CompanyStations[i]);
		}

		// Ship list
		TArray<IFlareShipInterface*>& CompanyShips = Target->GetCompanyShips();
		for (int32 i = 0; i < CompanyShips.Num(); i++)
		{
			if (CompanyShips[i]->IsAlive())
			{
				ShipList->AddShip(CompanyShips[i]);
			}
		}
	}

	ShipList->RefreshList();
}

void SFlareCompanyMenu::Exit()
{
	SetEnabled(false);
	ShipList->Reset();
	Company = NULL;
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FString SFlareCompanyMenu::GetCompanyName() const
{
	if (Company)
	{
		return Company->GetCompanyName();
	}
	else
	{
		return FString("<undefined>");
	}
}

void SFlareCompanyMenu::OnDashboardClicked()
{
	OwnerHUD->OpenMenu(EFlareMenu::MENU_Dashboard);
}



#undef LOCTEXT_NAMESPACE
