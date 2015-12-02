
#include "../../Flare.h"
#include "FlareTradeMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareRoundButton.h"
#include "../Components/FlareCargoInfo.h"

#define LOCTEXT_NAMESPACE "FlareTradeMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareTradeMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Padding(Theme.ContentPadding)
		[
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SImage).Image(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Trade))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TitleFont)
				.Text(this, &SFlareTradeMenu::GetTitle)
			]

			// Close
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("Back", "Back"))
				.HelpText(LOCTEXT("BackInfo", "Go to the previous menu"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Exit, true))
				.OnClicked(this, &SFlareTradeMenu::OnBackClicked)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 20))
		[
			SNew(SImage).Image(&Theme.SeparatorBrush)
		]

		// Content block
		+ SVerticalBox::Slot()
		.Padding(Theme.ContentPadding)
		.HAlign(HAlign_Center)
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			[
				SNew(SHorizontalBox)

				// Left spacecraft aka the current ship
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SNew(SBox)
					.HAlign(HAlign_Left)
					.WidthOverride(Theme.ContentWidth)
					[
						SNew(SVerticalBox)

						// Current ship's name
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(this, &SFlareTradeMenu::GetLeftSpacecraftName)
						]

						// Current ship's cargo
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(LeftCargoBay, SHorizontalBox)
						]
					]
				]

				// Right spacecraft aka the ship we're going to trade with
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				[
					SNew(SBox)
					.HAlign(HAlign_Left)
					.WidthOverride(Theme.ContentWidth)
					[
						SNew(SVerticalBox)

						// Ship's name
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(this, &SFlareTradeMenu::GetRightSpacecraftName)
							// TODO : Visible only when ship selected else Visibility::Collapsed, use SFlareShipList to select the target
						]
				
						// Ship's cargo
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SAssignNew(RightCargoBay, SHorizontalBox)
						]

						// TODO : a button to go back to selecting the ship
					]
				]

				// TODO : price, volume selection & confirmation box
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareTradeMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}

void SFlareTradeMenu::Enter(UFlareSimulatedSector* ParentSector, UFlareSimulatedSpacecraft* LeftSpacecraft, UFlareSimulatedSpacecraft* RightSpacecraft)
{
	FLOGV("SFlareTradeMenu::Enter ParentSector=%p LeftSpacecraft=%p RightSpacecraft=%p", ParentSector, LeftSpacecraft, RightSpacecraft);

	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	TargetSector = ParentSector;
	TargetLeftSpacecraft = LeftSpacecraft;
	TargetRightSpacecraft = RightSpacecraft;

	FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftCargoBay);
	FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightCargoBay);
}

void SFlareTradeMenu::FillTradeBlock(UFlareSimulatedSpacecraft* TargetSpacecraft, UFlareSimulatedSpacecraft* OtherSpacecraft, TSharedPtr<SHorizontalBox> CargoBay)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	CargoBay->ClearChildren();

	// Both spacecrafts are set
	if (TargetSpacecraft)
	{
		TArray<FFlareCargo>& SpacecraftCargoBay = TargetSpacecraft->GetCargoBay();
		for (int CargoIndex = 0; CargoIndex < SpacecraftCargoBay.Num(); CargoIndex++)
		{
			FFlareCargo* Cargo = &SpacecraftCargoBay[CargoIndex];
			CargoBay->AddSlot()
			[
				SNew(SFlareCargoInfo)
				.Spacecraft(TargetSpacecraft)
				.CargoIndex(CargoIndex)
				.OnClicked(this, &SFlareTradeMenu::OnTransferResources, TargetSpacecraft, OtherSpacecraft, Cargo->Resource, TSharedPtr<uint32>(new uint32(1)))
			];
		}
	}

	// Target isn't set, show the candidate's list
	else
	{
		TArray<UFlareSimulatedSpacecraft*>& SectorSpacecrafts = TargetSector->GetSectorSpacecrafts();

		for (int32 SpacecraftIndex = 0; SpacecraftIndex < SectorSpacecrafts.Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* SpacecraftCandidate = SectorSpacecrafts[SpacecraftIndex];
			if (!OtherSpacecraft->CanTradeWith(SpacecraftCandidate))
			{
				continue;
			}

			CargoBay->AddSlot()
			[
				SNew(SFlareButton)
				.Text(FText::FromName(SpacecraftCandidate->GetImmatriculation()))
				.OnClicked(this, &SFlareTradeMenu::OnSelectSpacecraft, SpacecraftCandidate)
			];
		}
	}
}

void SFlareTradeMenu::Exit()
{
	SetEnabled(false);

	TargetLeftSpacecraft = NULL;
	TargetRightSpacecraft = NULL;
	LeftCargoBay->ClearChildren();
	RightCargoBay->ClearChildren();

	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareTradeMenu::GetTitle() const
{
	return LOCTEXT("Trade", "TRADE");

	/*if (TargetSector)
	{
		Result = FText::Format(Result, FText::FromString(TargetSector->GetSectorName().ToString().ToUpper())); // FString needed here
	}

	return Result;*/
}

FText SFlareTradeMenu::GetLeftSpacecraftName() const
{
	if (TargetLeftSpacecraft)
	{
		return FText::FromName(TargetLeftSpacecraft->GetImmatriculation());
	}
	else
	{
		return FText();
	}
}

FText SFlareTradeMenu::GetRightSpacecraftName() const
{
	if (TargetRightSpacecraft)
	{
		return FText::FromName(TargetRightSpacecraft->GetImmatriculation());
	}
	else
	{
		return LOCTEXT("SelectSpacecraft", "Select a spacecraft to trade with");
	}
}


void SFlareTradeMenu::OnBackClicked()
{
	MenuManager->Back();
}

void SFlareTradeMenu::OnSelectSpacecraft(UFlareSimulatedSpacecraft*Spacecraft)
{
	if (TargetLeftSpacecraft)
	{
		TargetRightSpacecraft = Spacecraft;
	}
	else
	{
		TargetLeftSpacecraft = Spacecraft;
	}

	FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightCargoBay);
	FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftCargoBay);
}

void SFlareTradeMenu::OnTransferResources(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, TSharedPtr<uint32> Quantity)
{
	if (DestinationSpacecraft)
	{
		if (SourceSpacecraft && Resource)
		{
			TargetSector->GetGame()->GetGameWorld()->TransfertResources(SourceSpacecraft, DestinationSpacecraft, Resource, *Quantity.Get());
		}

		FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightCargoBay);
		FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftCargoBay);
	}
}

#undef LOCTEXT_NAMESPACE

