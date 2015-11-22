
#include "../../Flare.h"
#include "FlareTradeMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../Components/FlareButton.h"
#include "../Components/FlareRoundButton.h"

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
		[
			SNew(SScrollBox)
			.Style(&Theme.ScrollBoxStyle)
			.ScrollBarStyle(&Theme.ScrollBarStyle)

			+ SScrollBox::Slot()
			[
				SNew(SHorizontalBox)

				// Left spacecraft
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SAssignNew(LeftBlock, SVerticalBox)
				]

				// Right spacecraft
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				[
					SAssignNew(RightBlock, SVerticalBox)
				]


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
	FLOG("SFlareTradeMenu::Enter");

	FLOGV("SFlareTradeMenu::Enter ParentSector=%p LeftSpacecraft=%p RightSpacecraft=%p", ParentSector, LeftSpacecraft, RightSpacecraft);


	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	TargetSector = ParentSector;
	TargetLeftSpacecraft = LeftSpacecraft;
	TargetRightSpacecraft = RightSpacecraft;

	AFlarePlayerController* PC = MenuManager->GetPC();

	FLOGV("FillTradeBlock left %p", TargetLeftSpacecraft);
	FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftBlock);
	FLOGV("FillTradeBlock right %p", TargetRightSpacecraft);
	FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightBlock);
}

void SFlareTradeMenu::FillTradeBlock(UFlareSimulatedSpacecraft* TargetSpacecraft, UFlareSimulatedSpacecraft* OtherSpacecraft, TSharedPtr<SVerticalBox> TargetBlock)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	TargetBlock->ClearChildren();

	if(TargetSpacecraft == NULL)
	{
		//TODO ship selection menu
		TargetBlock->AddSlot()
		[
			SNew(STextBlock)
			.TextStyle(&Theme.SubTitleFont)
			.Text(FText::FromString(TEXT("Selected spacecraft to trade")))
		];

		TArray<UFlareSimulatedSpacecraft*>& SectorSpacecrafts = TargetSector->GetSectorSpacecrafts();

		for (int SpacecraftIndex = 0; SpacecraftIndex <  SectorSpacecrafts.Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* SpacecraftCandidate = SectorSpacecrafts[SpacecraftIndex];
			if(! OtherSpacecraft->CanTradeWith(SpacecraftCandidate))
			{
				continue;
			}

			TargetBlock->AddSlot()
			[
				SNew(SFlareButton)
				.Text(FText::FromName(SpacecraftCandidate->GetImmatriculation()))
				.OnClicked(this, &SFlareTradeMenu::OnSelectSpacecraft, SpacecraftCandidate)
			];
		}
	}
	else
	{
		TargetBlock->AddSlot()
		[
			SNew(STextBlock)
			.TextStyle(&Theme.SubTitleFont)
			.Text(FText::FromName(TargetSpacecraft->GetImmatriculation()))
		];


		TArray<FFlareCargo>& SpacecraftCargoBay = TargetSpacecraft->GetCargoBay();
		for (int CargoIndex = 0; CargoIndex < SpacecraftCargoBay.Num() ; CargoIndex++)
		{
			FFlareCargo* Cargo = &SpacecraftCargoBay[CargoIndex];

			FString ResourceString = FString::Printf(TEXT("- %s (%u/%u)"), (Cargo->Resource ? *Cargo->Resource->Name.ToString() : TEXT("[Empty]")), Cargo->Quantity, Cargo->Capacity);


			if(Cargo->Resource == NULL || !OtherSpacecraft)
			{
				TargetBlock->AddSlot()
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(FText::FromString(ResourceString))
				];
			}
			else
			{
				TargetBlock->AddSlot()
				[
						SNew(SFlareButton)
						.Text(FText::FromString(ResourceString))
						.OnClicked(this, &SFlareTradeMenu::OnTransferResources, TargetSpacecraft, OtherSpacecraft, Cargo->Resource, TSharedPtr<uint32>(new uint32(1)))
				];
			}





		}
	}
}

void SFlareTradeMenu::Exit()
{
	SetEnabled(false);
	TargetLeftSpacecraft = NULL;
	TargetRightSpacecraft = NULL;
	LeftBlock->ClearChildren();
	RightBlock->ClearChildren();
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

FText SFlareTradeMenu::GetTitle() const
{
	FText Result = LOCTEXT("Trade", "TRADE IN ");

	if (TargetSector)
	{
		Result = FText::FromString(Result.ToString() + TargetSector->GetSectorName().ToString().ToUpper());
	}

	return Result;
}

void SFlareTradeMenu::OnBackClicked()
{
	MenuManager->Back();
}

void SFlareTradeMenu::OnSelectSpacecraft(UFlareSimulatedSpacecraft*Spacecraft)
{
	if(TargetLeftSpacecraft)
	{
		TargetRightSpacecraft = Spacecraft;
	}
	else
	{
		TargetLeftSpacecraft = Spacecraft;
	}
	FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightBlock);
	FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftBlock);
}

void SFlareTradeMenu::OnTransferResources(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource, TSharedPtr<uint32> Quantity)
{
	if(SourceSpacecraft && DestinationSpacecraft && Resource)
	{
		TargetSector->GetGame()->GetGameWorld()->TransfertResources(SourceSpacecraft, DestinationSpacecraft, Resource, *Quantity.Get());
	}
	FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightBlock);
	FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftBlock);
}

#undef LOCTEXT_NAMESPACE

