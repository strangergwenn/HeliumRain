
#include "../../Flare.h"
#include "FlareStationMenu.h"
#include "../Widgets/FlarePartInfo.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareHUD.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareStationMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareStationMenu::Construct(const FArguments& InArgs)
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

					// Menu title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(10))
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SImage).Image(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Station))
						]

						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Title", "STATION"))
							.TextStyle(FFlareStyleSet::Get(), "Flare.Title1")
						]
					]

					// Section title
					+ SVerticalBox::Slot()
					.Padding(FMargin(10))
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Overview", "OVERVIEW"))
						.TextStyle(FFlareStyleSet::Get(), "Flare.Title2")
					]

					// Action box for this station
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(10))
					[
						SAssignNew(ObjectActionMenu, SFlareTargetActions)
						.Player(PC)
						.NoInspect(true)
					]

					// Object description
					+ SVerticalBox::Slot()
					.Padding(FMargin(10))
					.AutoHeight()
					[
						SAssignNew(ObjectDescription, STextBlock)
						.TextStyle(FFlareStyleSet::Get(), "Flare.Text")
						.WrapTextAt(600)
					]

					// Object list
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(ShipList, SFlareShipList)
						.OwnerHUD(OwnerHUD)
						.Title(LOCTEXT("DockedShips", "DOCKED SHIPS"))
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
			.OnClicked(this, &SFlareStationMenu::OnDashboardClicked)
		]
	];

	// Dashboard button
	BackButton->GetContainer()->SetContent(SNew(SImage).Image(AFlareHUD::GetMenuIcon(EFlareMenu::MENU_Exit)));
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareStationMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);	
}

void SFlareStationMenu::Enter(IFlareSpacecraftInterface* Target)
{
	FLOG("SFlareStationMenu::Enter");
	SetEnabled(true);

	CurrentStationTarget = Target;
	SetVisibility(EVisibility::Visible);
	ObjectActionMenu->SetStation(Target);
	ObjectActionMenu->Show();

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC && Target)
	{
		// Menu
		PC->GetMenuPawn()->SetHorizontalOffset(100);
		PC->GetMenuPawn()->UpdateBackgroundColor(0.1, 0.8);

		// Load station data
		FFlareSpacecraftSave* Data = Target->Save();
		if (Data)
		{
			FFlareSpacecraftDescription* Desc = PC->GetGame()->GetSpacecraftCatalog()->Get(Data->Identifier);
			if (Desc)
			{
				ObjectDescription->SetText(Desc->Description);
				PC->GetMenuPawn()->ShowShip(Desc, Data);
			}
		}

		// Fill the data
		TArray<IFlareSpacecraftInterface*> DockedShips = Target->GetDockedShips();
		for (int32 i = 0; i < DockedShips.Num(); i++)
		{
			if (DockedShips[i]->GetDamageSystem()->IsAlive())
			{
				ShipList->AddShip(DockedShips[i]);
			}
		}
	}

	ShipList->RefreshList();
}

void SFlareStationMenu::Exit()
{
	SetEnabled(false);
	ShipList->Reset();
	ObjectActionMenu->Hide();
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareStationMenu::OnDashboardClicked()
{
	OwnerHUD->OpenMenu(EFlareMenu::MENU_Dashboard);
}

#undef LOCTEXT_NAMESPACE
