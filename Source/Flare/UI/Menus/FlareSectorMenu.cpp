
#include "../../Flare.h"
#include "FlareSectorMenu.h"

#include "../../Game/FlareGame.h"
#include "../../Player/FlareHUD.h"
#include "../../Player/FlarePlayerController.h"

#include "../Components/FlareDashboardButton.h"

#define LOCTEXT_NAMESPACE "FlareSectorMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSectorMenu::Construct(const FArguments& InArgs)
{
	// Data
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
							SNew(SImage).Image(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Sector))
						]

						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Sector", "SECTOR MAP"))
							.TextStyle(FFlareStyleSet::Get(), "Flare.Title1")
						]
					]

					// Object list
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(ShipList, SFlareShipList)
						.OwnerHUD(OwnerHUD)
						.Title(LOCTEXT("SectorTargetListTitle", "OBJECTS IN SECTOR"))
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
			.OnClicked(this, &SFlareSectorMenu::OnDashboardClicked)
		]
	];

	// SectorMenu close button
	BackButton->GetContainer()->SetContent(SNew(SImage).Image(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Exit)));
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareSectorMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}

void SFlareSectorMenu::Enter()
{
	FLOG("SFlareSectorMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
		PC->GetMenuPawn()->UpdateBackgroundColor(0.15, 0.15);

		for (TActorIterator<AActor> ActorItr(PC->GetWorld()); ActorItr; ++ActorItr)
		{
			// Ship
			AFlareSpacecraft* ShipCandidate = Cast<AFlareSpacecraft>(*ActorItr);
			if (ShipCandidate && ShipCandidate->GetDamageSystem()->IsAlive())
			{
				ShipList->AddShip(ShipCandidate);
			}
		}
	}

	ShipList->RefreshList();
}

void SFlareSectorMenu::Exit()
{
	SetEnabled(false);
	ShipList->Reset();
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareSectorMenu::OnDashboardClicked()
{
	OwnerHUD->OpenMenu(EFlareMenu::MENU_Dashboard);
}

#undef LOCTEXT_NAMESPACE

