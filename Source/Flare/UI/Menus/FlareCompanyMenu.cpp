
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

					// Section title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(10))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("DockedShips", "DOCKED SHIPS"))
						.TextStyle(FFlareStyleSet::Get(), "Flare.Title2")
					]

					//// Station box
					//+ SVerticalBox::Slot()
					//.AutoHeight()
					//.Padding(FMargin(10))
					//[
					//	SAssignNew(TargetList, SListView< TSharedPtr<FInterfaceContainer> >)
					//	.ListItemsSource(&TargetListData)
					//	.SelectionMode(ESelectionMode::Single)
					//	.OnGenerateRow(this, &SFlareCompanyMenu::GenerateTargetInfo)
					//	.OnSelectionChanged(this, &SFlareCompanyMenu::OnTargetSelected)
					//]

					//// Section title
					//+ SVerticalBox::Slot()
					//.AutoHeight()
					//.Padding(FMargin(10))
					//[
					//	SAssignNew(SelectedTargetText, STextBlock)
					//	.Text(LOCTEXT("SelectedShip", "SELECTED SHIP"))
					//	.TextStyle(FFlareStyleSet::Get(), "Flare.Title2")
					//]

					//// Action box
					//+ SVerticalBox::Slot()
					//.AutoHeight()
					//.Padding(FMargin(10))
					//[
					//	SAssignNew(ActionMenu, SFlareTargetActions)
					//	.Player(PC)
					//]
				]
			]
		]

		// Color picker
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		[
			SNew(SVerticalBox)

			// Color title
			+ SVerticalBox::Slot()
			.Padding(FMargin(0, 20))
			.AutoHeight()
			[
				SAssignNew(ColorBoxTitle, STextBlock)
				.Text(LOCTEXT("ShipPartsColor", "PAINT SCHEME"))
				.TextStyle(FFlareStyleSet::Get(), "Flare.Title2")
			]

			// Picker
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(ColorBox, SFlareColorPanel).OwnerHUD(OwnerHUD)
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
	ColorBox->Setup(PlayerData);
	SetVisibility(EVisibility::Hidden);	
}

void SFlareCompanyMenu::Enter(UFlareCompany* Target)
{
	FLOG("SFlareCompanyMenu::Enter");

	SetVisibility(EVisibility::Visible);

	// Move the viewer to the right
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
		PC->GetMenuPawn()->SetHorizontalOffset(100);
		PC->GetMenuPawn()->UpdateBackgroundColor(0.85, 1.0);
	}

}

void SFlareCompanyMenu::Exit()
{
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareCompanyMenu::OnDashboardClicked()
{
	OwnerHUD->OpenMenu(EFlareMenu::MENU_Dashboard);
}



#undef LOCTEXT_NAMESPACE
