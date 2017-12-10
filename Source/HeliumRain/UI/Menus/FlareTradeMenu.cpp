
#include "FlareTradeMenu.h"

#include "../../Flare.h"

#include "../../Data/FlareResourceCatalog.h"

#include "../../Game/FlareGame.h"
#include "../../Game/FlareSectorHelper.h"
#include "../../Game/FlareGameTools.h"

#include "../../Economy/FlareCargoBay.h"

#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#include "../Components/FlareRoundButton.h"
#include "../Components/FlareCargoInfo.h"
#include "../../Quests/FlareQuest.h"
#include "../../Quests/FlareQuestStep.h"
#include "../../Quests/FlareQuestCondition.h"

#define LOCTEXT_NAMESPACE "FlareTradeMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareTradeMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	int32 TextWidth = Theme.ContentWidth - 2 * Theme.ContentPadding.Left - 2 * Theme.ContentPadding.Right;

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SVerticalBox)

		// Content block
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Fill)
		[
			SNew(SHorizontalBox)

			// Left spacecraft aka the current ship
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			[
				SNew(SScrollBox)
				.Style(&Theme.ScrollBoxStyle)
				.ScrollBarStyle(&Theme.ScrollBarStyle)

				+ SScrollBox::Slot()
				[
					SNew(SBox)
					.HAlign(HAlign_Left)
					.WidthOverride(Theme.ContentWidth)
					[
						SNew(SVerticalBox)

						// Current ship's name
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(this, &SFlareTradeMenu::GetLeftSpacecraftName)
						]

						// Current ship's cargo 1
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						.HAlign(HAlign_Left)
						[
							SAssignNew(LeftCargoBay1, SHorizontalBox)
						]

						// Current ship's cargo 2
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						.HAlign(HAlign_Left)
						[
							SAssignNew(LeftCargoBay2, SHorizontalBox)
						]

						// Help text
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(LOCTEXT("HelpText", "Your ship is ready to trade with another spacecraft."))
							.WrapTextAt(TextWidth)
						]
					]
				]
			]

			// Right spacecraft aka the ship we're going to trade with
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			[
				SNew(SBox)
				.HAlign(HAlign_Fill)
				.WidthOverride(Theme.ContentWidth)
				[
					SNew(SScrollBox)
					.Style(&Theme.ScrollBoxStyle)
					.ScrollBarStyle(&Theme.ScrollBarStyle)

					+ SScrollBox::Slot()
					[
						SNew(SVerticalBox)
						
						// Ship selection list
						+ SVerticalBox::Slot()
						[
							SAssignNew(ShipList, SFlareList)
							.MenuManager(MenuManager)
							.Title(LOCTEXT("SelectSpacecraft", "Select a spacecraft to trade with"))
							.OnItemSelected(this, &SFlareTradeMenu::OnSpacecraftSelected)
						]

						// Ship's name
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(this, &SFlareTradeMenu::GetRightSpacecraftName)
							.Visibility(this, &SFlareTradeMenu::GetTradingVisibility)
							.WrapTextAt(TextWidth)
						]
				
						// Ship's cargo
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Fill)
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SVerticalBox)

								// Current ship's cargo 1
								+ SVerticalBox::Slot()
								.Padding(Theme.SmallContentPadding)
								.AutoHeight()
								.HAlign(HAlign_Left)
								[
									SAssignNew(RightCargoBay1, SHorizontalBox)
									.Visibility(this, &SFlareTradeMenu::GetTradingVisibility)
								]

								// Current ship's cargo 2
								+ SVerticalBox::Slot()
								.Padding(Theme.SmallContentPadding)
								.AutoHeight()
								.HAlign(HAlign_Left)
								[
									SAssignNew(RightCargoBay2, SHorizontalBox)
									.Visibility(this, &SFlareTradeMenu::GetTradingVisibility)
								]
							]

							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Right)
							.VAlign(VAlign_Center)
							.Padding(Theme.SmallContentPadding)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("BackToSelection", "Change target"))
								.Icon(FFlareStyleSet::GetIcon("Stop"))
								.OnClicked(this, &SFlareTradeMenu::OnBackToSelection)
								.Visibility(this, &SFlareTradeMenu::GetBackToSelectionVisibility)
								.Width(4)
							]
						]

						// Construction text
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(LOCTEXT("ConstructionInfo", "This station is under construction and needs resources to be completed."))
							.Visibility(this, &SFlareTradeMenu::GetConstructionInfosVisibility)
							.WrapTextAt(TextWidth)
						]

						// Help text
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(LOCTEXT("OtherShipHelpText", "Click on a resource to start trading."))
							.Visibility(this, &SFlareTradeMenu::GetTradingVisibility)
							.WrapTextAt(TextWidth)
						]

						// Price box
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SBox)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Top)
							.WidthOverride(Theme.ContentWidth)
							[
								SNew(SVerticalBox)

								// Title
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(Theme.TitlePadding)
								[
									SNew(STextBlock)
									.TextStyle(&Theme.SubTitleFont)
									.Text(LOCTEXT("TransactionTitle", "Transaction details"))
									.Visibility(this, &SFlareTradeMenu::GetTransactionDetailsVisibility)
									.WrapTextAt(TextWidth)
								]

								// Invalid transaction
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(Theme.ContentPadding)
								[
									SNew(STextBlock)
									.TextStyle(&Theme.NameFont)
									.Text(this, &SFlareTradeMenu::GetTransactionInvalidDetails)
									.Visibility(this, &SFlareTradeMenu::GetTransactionInvalidVisibility)
									.WrapTextAt(TextWidth)
								]

								// Quantity
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SHorizontalBox)

									// Slider
									+ SHorizontalBox::Slot()
									.HAlign(HAlign_Fill)
									.Padding(Theme.ContentPadding)
									[
										SAssignNew(QuantitySlider, SSlider)
										.Style(&Theme.SliderStyle)
										.Value(0)
										.OnValueChanged(this, &SFlareTradeMenu::OnResourceQuantityChanged)
										.Visibility(this, &SFlareTradeMenu::GetTransactionDetailsVisibility)
									]

									// Text box
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.HAlign(HAlign_Right)
									.Padding(Theme.ContentPadding)
									[
										SAssignNew(QuantityText, SEditableText)
										.AllowContextMenu(false)
										.Style(&Theme.TextInputStyle)
										.OnTextChanged(this, &SFlareTradeMenu::OnResourceQuantityEntered)
										.Visibility(this, &SFlareTradeMenu::GetTransactionDetailsVisibility)
									]
								]

								// Info
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(Theme.ContentPadding)
								[
									SNew(STextBlock)
									.TextStyle(&Theme.TextFont)
									.Text(this, &SFlareTradeMenu::GetTransactionDetails)
									.Visibility(this, &SFlareTradeMenu::GetTransactionDetailsVisibility)
									.WrapTextAt(TextWidth)
								]

								// Price
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SAssignNew(PriceBox, SFlareConfirmationBox)
									.ConfirmText(LOCTEXT("Confirm", "Confirm transfer"))
									.CancelText(LOCTEXT("BackTopShip", "Cancel"))
									.OnConfirmed(this, &SFlareTradeMenu::OnConfirmTransaction)
									.OnCancelled(this, &SFlareTradeMenu::OnCancelTransaction)
									.TradeBehavior(true)
									.PC(MenuManager->GetPC())
								]
							]
						]
					]
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
	SetVisibility(EVisibility::Collapsed);
	ShipList->SetVisibility(EVisibility::Collapsed);
}

void SFlareTradeMenu::Enter(UFlareSimulatedSector* ParentSector, UFlareSimulatedSpacecraft* LeftSpacecraft, UFlareSimulatedSpacecraft* RightSpacecraft)
{
	FLOGV("SFlareTradeMenu::Enter ParentSector=%p LeftSpacecraft=%p RightSpacecraft=%p", ParentSector, LeftSpacecraft, RightSpacecraft);
	
	// Setup
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);
	TargetSector = ParentSector;
	TargetLeftSpacecraft = LeftSpacecraft;
	ShipList->Reset();
	WasActiveSector = false;

	// First-person trading override
	AFlareSpacecraft* PhysicalSpacecraft = TargetLeftSpacecraft->GetActive();

	if (TargetLeftSpacecraft->IsActive())
	{
		WasActiveSector = true;
		if (PhysicalSpacecraft->GetNavigationSystem()->IsDocked())
		{
			TargetRightSpacecraft = PhysicalSpacecraft->GetNavigationSystem()->GetDockStation()->GetParent();
		}
	}
	else
	{
		TargetRightSpacecraft = RightSpacecraft;
	}

	// Not first person - list spacecrafts
	if (TargetLeftSpacecraft->GetCurrentFleet() != MenuManager->GetPC()->GetPlayerFleet())
	{
		// Add stations
		for (int32 SpacecraftIndex = 0; SpacecraftIndex < ParentSector->GetSectorStations().Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* StationCandidate = ParentSector->GetSectorStations()[SpacecraftIndex];
			if (StationCandidate && StationCandidate != LeftSpacecraft && StationCandidate != RightSpacecraft
			 && StationCandidate->GetActiveCargoBay()->GetSlotCount() > 0
			 && StationCandidate->GetCompany()->GetPlayerWarState() != EFlareHostility::Hostile)
			{
				ShipList->AddShip(StationCandidate);
			}
		}

		// Add ships
		for (int32 SpacecraftIndex = 0; SpacecraftIndex < ParentSector->GetSectorShips().Num(); SpacecraftIndex++)
		{
			// Don't allow trade with other
			UFlareSimulatedSpacecraft* ShipCandidate = ParentSector->GetSectorShips()[SpacecraftIndex];
			if (ShipCandidate && ShipCandidate != LeftSpacecraft && ShipCandidate != RightSpacecraft
			 && ShipCandidate->GetActiveCargoBay()->GetSlotCount() > 0
			 && ShipCandidate->GetDamageSystem()->IsAlive()
			 && ShipCandidate->GetCompany() == MenuManager->GetPC()->GetCompany())
			{
				ShipList->AddShip(ShipCandidate);
			}
		}
	}

	// Setup widgets
	AFlarePlayerController* PC = MenuManager->GetPC();
	FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftCargoBay1, LeftCargoBay2);
	FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightCargoBay1, RightCargoBay2);
	ShipList->RefreshList();

	// Show selector if still needed
	if (TargetRightSpacecraft)
	{
		ShipList->SetVisibility(EVisibility::Collapsed);
	}
	else
	{
		ShipList->SetVisibility(EVisibility::Visible);
	}
}

void SFlareTradeMenu::FillTradeBlock(UFlareSimulatedSpacecraft* TargetSpacecraft, UFlareSimulatedSpacecraft* OtherSpacecraft,
	TSharedPtr<SHorizontalBox> CargoBay1, TSharedPtr<SHorizontalBox> CargoBay2)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	// Prepare cargo bay
	CargoBay1->ClearChildren();
	CargoBay2->ClearChildren();
	TArray<FSortableCargoInfo> SortedCargoBay;

	// Both spacecrafts are set
	if (TargetSpacecraft)
	{
		// Get slots
		for (int32 CargoIndex = 0; CargoIndex < TargetSpacecraft->GetActiveCargoBay()->GetSlotCount(); CargoIndex++)
		{
			FFlareCargo* Cargo = TargetSpacecraft->GetActiveCargoBay()->GetSlot(CargoIndex);
			FSortableCargoInfo CargoInfo;
			CargoInfo.Cargo = Cargo;
			CargoInfo.CargoInitialIndex = CargoIndex;
			SortedCargoBay.Add(CargoInfo);
		}

		// Sort and fill
		SortedCargoBay.Sort(UFlareCargoBay::SortBySlotType);
		for (int32 CargoIndex = 0; CargoIndex < SortedCargoBay.Num(); CargoIndex++)
		{
			TSharedPtr<SHorizontalBox> Bay = (CargoIndex < 8) ? CargoBay1 : CargoBay2;
			Bay->AddSlot()
			[
				SNew(SFlareCargoInfo)
				.Spacecraft(TargetSpacecraft)
				.CargoIndex(SortedCargoBay[CargoIndex].CargoInitialIndex)
				.OnClicked(this, &SFlareTradeMenu::OnTransferResources,
					TargetSpacecraft,
					OtherSpacecraft,
					SortedCargoBay[CargoIndex].Cargo->Resource)
			];
		}
	}
}

void SFlareTradeMenu::Exit()
{
	SetEnabled(false);

	// Reset cargo
	TargetLeftSpacecraft = NULL;
	TargetRightSpacecraft = NULL;
	LeftCargoBay1->ClearChildren();
	LeftCargoBay2->ClearChildren();
	RightCargoBay1->ClearChildren();
	RightCargoBay2->ClearChildren();

	// Reset transaction data
	TransactionDestinationSpacecraft = NULL;
	TransactionSourceSpacecraft = NULL;
	TransactionResource = NULL;

	// Reset menus
	PriceBox->Hide();
	ShipList->Reset();
	ShipList->SetVisibility(EVisibility::Collapsed);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareTradeMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (IsEnabled() && !WasActiveSector)
	{
		if (TargetLeftSpacecraft->IsActive())
		{
			Enter(TargetSector, TargetLeftSpacecraft, TargetRightSpacecraft);
		}
	}
}

/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

EVisibility SFlareTradeMenu::GetTradingVisibility() const
{
	return (TargetLeftSpacecraft && TargetRightSpacecraft) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlareTradeMenu::GetConstructionInfosVisibility() const
{
	return (TargetRightSpacecraft && TargetRightSpacecraft->IsStation() && TargetRightSpacecraft->IsUnderConstruction()) ? EVisibility::Visible : EVisibility::Collapsed;
}


EVisibility SFlareTradeMenu::GetBackToSelectionVisibility() const
{
	if (!IsEnabled())
	{
		return EVisibility::Collapsed;
	}

	// First-person trading override
	AFlareSpacecraft* PhysicalSpacecraft = TargetLeftSpacecraft->GetActive();
	if (PhysicalSpacecraft && PhysicalSpacecraft->GetNavigationSystem()->IsDocked())
	{
		return EVisibility::Collapsed;
	}

	else
	{
		return TargetRightSpacecraft ? EVisibility::Visible : EVisibility::Collapsed;
	}
}

EVisibility SFlareTradeMenu::GetTransactionDetailsVisibility() const
{
	FText Unused;
	return IsEnabled() && IsTransactionValid(Unused) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlareTradeMenu::GetTransactionInvalidVisibility() const
{
	FText Unused;
	return (IsEnabled() && !IsTransactionValid(Unused) && TransactionSourceSpacecraft && TransactionDestinationSpacecraft && TransactionResource) ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SFlareTradeMenu::GetLeftSpacecraftName() const
{
	if (TargetLeftSpacecraft)
	{
		return UFlareGameTools::DisplaySpacecraftName(TargetLeftSpacecraft);
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
		return UFlareGameTools::DisplaySpacecraftName(TargetRightSpacecraft);
	}
	else
	{
		return LOCTEXT("NoSelectedSpacecraft", "No spacecraft selected");
	}
}

FText SFlareTradeMenu::GetTransactionDetails() const
{
	if (TransactionSourceSpacecraft && TransactionDestinationSpacecraft && TransactionResource)
	{
		FText MainInfo = FText::Format(LOCTEXT("TradeInfoFormat", "Trading {0}x {1} from {2} to {3}."),
			FText::AsNumber(TransactionQuantity),
			TransactionResource->Name,
			UFlareGameTools::DisplaySpacecraftName(TransactionSourceSpacecraft),
			UFlareGameTools::DisplaySpacecraftName(TransactionDestinationSpacecraft));

		FText UnitPrice;
		FText AffordableInfo;

		if (TransactionSourceSpacecraft->GetCompany() != MenuManager->GetPC()->GetCompany()
				|| TransactionDestinationSpacecraft->GetCompany() != MenuManager->GetPC()->GetCompany())
		{
			int64 BaseResourcePrice = TransactionSourceSpacecraft->GetResourcePrice(TransactionResource, EFlareResourcePriceContext::Default);
			int64 TransactionResourcePrice = TransactionSourceSpacecraft->GetCurrentSector()->GetTransfertResourcePrice(TransactionSourceSpacecraft, TransactionDestinationSpacecraft, TransactionResource);

			int64 Fee = TransactionResourcePrice - BaseResourcePrice;

			if(TransactionDestinationSpacecraft->GetCompany() == MenuManager->GetPC()->GetCompany())
			{
				UnitPrice = FText::Format(LOCTEXT("PurchaseUnitPriceFormat", "\nPurchase price: {0} credits/unit ({1} {2} {3} transport fee)"),
					UFlareGameTools::DisplayMoney(TransactionResourcePrice),
					UFlareGameTools::DisplayMoney(BaseResourcePrice),
					(Fee < 0 ? LOCTEXT("Minus", "-"): LOCTEXT("Plus", "+")),
					UFlareGameTools::DisplayMoney(FMath::Abs(Fee)));
			}
			else
			{
				UnitPrice = FText::Format(LOCTEXT("SellUnitPriceFormat", "\nSell price: {0} credits/unit ({1} {2} {3} transport fee)"),
					UFlareGameTools::DisplayMoney(TransactionResourcePrice),
					UFlareGameTools::DisplayMoney(BaseResourcePrice),
					(Fee < 0 ? LOCTEXT("Minus", "-"): LOCTEXT("Plus", "+")),
					UFlareGameTools::DisplayMoney(FMath::Abs(Fee)));
			}
		}

		// Add buyer capability if it's not the player
		if (TransactionDestinationSpacecraft->GetCompany() != MenuManager->GetPC()->GetCompany())
		{
			AffordableInfo = FText::Format(LOCTEXT("TradeAffordableFormat", "\nThe buyer has {0} credits available."),
				UFlareGameTools::DisplayMoney(TransactionDestinationSpacecraft->GetCompany()->GetMoney()));
		}

		return FText::Format(LOCTEXT("TradeInfoMergeFormat", "{0}{1}{2}"), MainInfo, UnitPrice, AffordableInfo);
	}
	else
	{
		return FText();
	}
}

FText SFlareTradeMenu::GetTransactionInvalidDetails() const
{
	if (TransactionSourceSpacecraft && TransactionDestinationSpacecraft && TransactionResource)
	{
		FText Reason;
		IsTransactionValid(Reason);

		if (Reason.IsEmptyOrWhitespace())
		{
			Reason = LOCTEXT("TradeInvalidDefaultError", "The buyer needs an empty slot, or one with the matching resource.\n\u2022 Input resources are never sold.\n\u2022 Output resources are never bought");
		}

		return FText::Format(LOCTEXT("TradeInvalidInfoFormat", "Can't trade {0} from {1} to {2} !\n\u2022 {3}"),
			TransactionResource->Name,
			UFlareGameTools::DisplaySpacecraftName(TransactionSourceSpacecraft),
			UFlareGameTools::DisplaySpacecraftName(TransactionDestinationSpacecraft),
			Reason);
	}
	else
	{
		return FText();
	}
}

FText SFlareTradeMenu::GetResourcePriceInfo(FFlareResourceDescription* Resource) const
{
	if (TargetSector)
	{
		FNumberFormattingOptions MoneyFormat;
		MoneyFormat.MaximumFractionalDigits = 2;


		int32 MeanDuration = 50;
		int64 ResourcePrice = TargetSector->GetResourcePrice(Resource, EFlareResourcePriceContext::Default);
		int64 LastResourcePrice = TargetSector->GetResourcePrice(Resource, EFlareResourcePriceContext::Default, MeanDuration - 1);

		FText VariationText;


		if(ResourcePrice != LastResourcePrice)
		{
			float Variation = (((float) ResourcePrice) / ((float) LastResourcePrice) - 1);

			VariationText = UFlareGameTools::AddLeadingSpace(FText::Format(LOCTEXT("ResourceVariationFormat", "({0}{1}%)"),
							(Variation > 0 ?
								 LOCTEXT("ResourceVariationFormatSignPlus","+") :
								 LOCTEXT("ResourceVariationFormatSignMinus","-")),
						  FText::AsNumber(FMath::Abs(Variation) * 100.0f, &MoneyFormat)));
		}

		return FText::Format(LOCTEXT("ResourceMainPriceFormat", "{0} credits{2} - Transport fee : {1} credits "),
			FText::AsNumber(ResourcePrice / 100.0f, &MoneyFormat),
			FText::AsNumber(Resource->TransportFee / 100.0f, &MoneyFormat),
			VariationText);
	}

	return FText();
}

void SFlareTradeMenu::OnSpacecraftSelected(TSharedPtr<FInterfaceContainer> SpacecraftContainer)
{
	UFlareSimulatedSpacecraft* Spacecraft = SpacecraftContainer->SpacecraftPtr;

	if (Spacecraft)
	{
		// Store spacecrafts
		if (TargetLeftSpacecraft)
		{
			TargetRightSpacecraft = Spacecraft;
		}
		else
		{
			TargetLeftSpacecraft = Spacecraft;
		}

		// Reset menus
		PriceBox->Hide();
		FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightCargoBay1, RightCargoBay2);
		FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftCargoBay1, LeftCargoBay2);
		ShipList->SetVisibility(EVisibility::Collapsed);
	}
}

void SFlareTradeMenu::OnTransferResources(UFlareSimulatedSpacecraft* SourceSpacecraft, UFlareSimulatedSpacecraft* DestinationSpacecraft, FFlareResourceDescription* Resource)
{
	FLOGV("OnTransferResources %p %p", SourceSpacecraft, DestinationSpacecraft);
	if (DestinationSpacecraft)
	{
		// Store transaction data
		TransactionSourceSpacecraft = SourceSpacecraft;
		TransactionDestinationSpacecraft = DestinationSpacecraft;
		TransactionResource = Resource;
		TransactionQuantity = GetMaxTransactionAmount();

		QuantitySlider->SetValue(1.0f);
		QuantityText->SetText(FText::AsNumber(TransactionQuantity));

		UpdatePrice();


		UFlareQuest* SelectedQuest = MenuManager->GetGame()->GetQuestManager()->GetSelectedQuest();

		if (SelectedQuest && SelectedQuest->GetCurrentStep())
		{
			for(UFlareQuestCondition* Condition : SelectedQuest->GetCurrentConditions())
			{
				UFlareQuestConditionSellAtStation* SellAtStationCondition = Cast<UFlareQuestConditionSellAtStation>(Condition);
				if (SellAtStationCondition && SellAtStationCondition->GetResource() == Resource)
				{
					int32 MissingQuantity = SellAtStationCondition->GetTargetQuantity() - SellAtStationCondition->GetCurrentProgression();
					auto PlayerShip = (SourceSpacecraft->IsStation() ? DestinationSpacecraft : SourceSpacecraft);
					int32 PreferredQuantity = MissingQuantity - PlayerShip->GetActiveCargoBay()->GetResourceQuantity(Resource, PlayerShip->GetCompany());
				
					if (TransactionSourceSpacecraft == PlayerShip)
					{
						PreferredQuantity = MissingQuantity;
					}

					int32 AssignedQuantity = SetSliderQuantity(PreferredQuantity);
					QuantityText->SetText(FText::AsNumber(AssignedQuantity));
				}
			}
		}
	}
}

void SFlareTradeMenu::OnResourceQuantityChanged(float Value)
{
	int32 ResourceMaxQuantity = GetMaxTransactionAmount();

	// Calculate transaction amount, depending on max value (step mechanism)
	TransactionQuantity = FMath::Lerp((int32)1, ResourceMaxQuantity, Value);
	if (ResourceMaxQuantity >= 1000 && (TransactionQuantity - ResourceMaxQuantity) > 50)
	{
		TransactionQuantity = (TransactionQuantity / 50) * 50;
	}
	else if (ResourceMaxQuantity >= 100 && (TransactionQuantity - ResourceMaxQuantity) > 10)
	{
		TransactionQuantity = (TransactionQuantity / 10) * 10;
	}
	
	// Force slider value, update quantity
	if (ResourceMaxQuantity == 1)
	{
		QuantitySlider->SetValue(1.0f);
	}
	else
	{
		QuantitySlider->SetValue((float)(TransactionQuantity - 1) / (float)(ResourceMaxQuantity - 1));
	}

	QuantityText->SetText(FText::AsNumber(TransactionQuantity));

	UpdatePrice();
}

void SFlareTradeMenu::OnResourceQuantityEntered(const FText& TextValue)
{
	if (TextValue.ToString().IsNumeric())
	{
		SetSliderQuantity(FCString::Atoi(*TextValue.ToString()));
	}
}

void SFlareTradeMenu::OnConfirmTransaction()
{
	// Actual transaction
	if (TransactionSourceSpacecraft && TransactionSourceSpacecraft->GetCurrentSector() && TransactionDestinationSpacecraft && TransactionResource)
	{
		SectorHelper::Trade(TransactionSourceSpacecraft,
			TransactionDestinationSpacecraft,
			TransactionResource,
			TransactionQuantity);
	}

	// Reset transaction data
	TransactionDestinationSpacecraft = NULL;
	TransactionSourceSpacecraft = NULL;
	TransactionResource = NULL;
	TransactionQuantity = 0;

	// Reset menus
	PriceBox->Hide();
	FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightCargoBay1, RightCargoBay2);
	FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftCargoBay1, LeftCargoBay2);
	MenuManager->GetPC()->ClientPlaySound(MenuManager->GetPC()->GetSoundManager()->InfoSound);
}

void SFlareTradeMenu::OnCancelTransaction()
{
	// Reset transaction data
	TransactionDestinationSpacecraft = NULL;
	TransactionSourceSpacecraft = NULL;
	TransactionResource = NULL;
	TransactionQuantity = 0;

	// Reset menus
	PriceBox->Hide();
	FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightCargoBay1, RightCargoBay2);
	FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftCargoBay1, LeftCargoBay2);
	MenuManager->GetPC()->ClientPlaySound(MenuManager->GetPC()->GetSoundManager()->NegativeClickSound);
}

void SFlareTradeMenu::OnBackToSelection()
{
	TargetRightSpacecraft = NULL;

	// Reset transaction data
	TransactionDestinationSpacecraft = NULL;
	TransactionSourceSpacecraft = NULL;
	TransactionResource = NULL;
	TransactionQuantity = 0;

	// Reset menus
	PriceBox->Hide();
	RightCargoBay1->ClearChildren();
	RightCargoBay2->ClearChildren();
	ShipList->ClearSelection();
	ShipList->SetVisibility(EVisibility::Visible);
}

void SFlareTradeMenu::UpdatePrice()
{
	// Update price
	uint32 ResourceUnitPrice = 0;

	if (TransactionSourceSpacecraft && TransactionSourceSpacecraft->GetCurrentSector())
	{
		ResourceUnitPrice = TransactionSourceSpacecraft->GetCurrentSector()->GetTransfertResourcePrice(
			TransactionSourceSpacecraft,
			TransactionDestinationSpacecraft,
			TransactionResource);
	}

	FText Unused;
	if (IsTransactionValid(Unused))
	{
		if (TransactionSourceSpacecraft && TransactionDestinationSpacecraft->GetCompany() != TransactionSourceSpacecraft->GetCompany() && ResourceUnitPrice > 0)
		{
			int64 TransactionPrice = TransactionQuantity * ResourceUnitPrice;
			bool AllowDepts = TransactionDestinationSpacecraft->GetResourceUseType(TransactionResource).HasUsage(EFlareResourcePriceContext::ConsumerConsumption) || MenuManager->GetGame()->GetQuestManager()->IsTradeQuestUseStation(TransactionDestinationSpacecraft);

			PriceBox->Show(TransactionPrice, TransactionDestinationSpacecraft->GetCompany(), AllowDepts);
		}
		else
		{
			PriceBox->Show(0, MenuManager->GetPC()->GetCompany());
		}
	}
	else
	{
		PriceBox->Hide();
	}
}

int32 SFlareTradeMenu::SetSliderQuantity(int32 Quantity)
{
	int32 ResourceMaxQuantity = GetMaxTransactionAmount();

	TransactionQuantity = FMath::Clamp(Quantity, 0, ResourceMaxQuantity);
	FLOGV("SFlareTradeMenu::SetSliderQuantity number %d / %d", TransactionQuantity, ResourceMaxQuantity)

	if (ResourceMaxQuantity == 1)
	{
		QuantitySlider->SetValue(1.0f);
	}
	else
	{
		QuantitySlider->SetValue((float)(TransactionQuantity - 1) / (float)(ResourceMaxQuantity - 1));
	}

	UpdatePrice();

	return TransactionQuantity;
}

bool SFlareTradeMenu::IsTransactionValid(FText& Reason) const
{
	// Possible transaction
	if (TransactionSourceSpacecraft && TransactionDestinationSpacecraft && TransactionResource)
	{
		int32 ResourcePrice = TransactionSourceSpacecraft->GetCurrentSector()->GetTransfertResourcePrice(TransactionSourceSpacecraft, TransactionDestinationSpacecraft, TransactionResource);
		int32 MaxAffordableQuantity =  FMath::Max(int64(0), TransactionDestinationSpacecraft->GetCompany()->GetMoney() / ResourcePrice);
		int32 ResourceMaxQuantity = FMath::Min(TransactionSourceSpacecraft->GetActiveCargoBay()->GetResourceQuantity(TransactionResource, MenuManager->GetPC()->GetCompany()),
			TransactionDestinationSpacecraft->GetActiveCargoBay()->GetFreeSpaceForResource(TransactionResource, MenuManager->GetPC()->GetCompany()));

		// Special exception for same company
		if (TransactionSourceSpacecraft->GetCompany() == TransactionDestinationSpacecraft->GetCompany())
		{
			return true;
		}

		// Cases of failure + reason
		else if (!TransactionSourceSpacecraft->CanTradeWith(TransactionDestinationSpacecraft, Reason))
		{
			return false;
		}
		else if (!TransactionSourceSpacecraft->GetActiveCargoBay()->WantSell(TransactionResource, MenuManager->GetPC()->GetCompany()))
		{
			Reason = LOCTEXT("CantTradeSell", "This resource isn't sold by the seller. Input resources are never sold.");
			return false;
		}
		else if (!TransactionDestinationSpacecraft->GetActiveCargoBay()->WantBuy(TransactionResource, MenuManager->GetPC()->GetCompany()))
		{
			Reason = LOCTEXT("CantTradeBuy", "This resource isn't bought by the buyer. Output resources are never bought. The buyer needs an empty slot, or one with the matching resource.");
			return false;
		}
		else if (MaxAffordableQuantity == 0 && !MenuManager->GetGame()->GetQuestManager()->IsTradeQuestUseStation(TransactionDestinationSpacecraft))
		{

			if(TransactionDestinationSpacecraft->GetCompany() == MenuManager->GetPC()->GetCompany())
			{
				Reason = LOCTEXT("YouCantTradePrice", "You can't afford to buy any of this resource.");
			}
			else
			{
				Reason = LOCTEXT("CantTradePrice", "The buyer can't afford to buy any of this resource.");
			}

			return false;
		}
		else if (ResourceMaxQuantity == 0)
		{
			if(TransactionDestinationSpacecraft->GetCompany() == MenuManager->GetPC()->GetCompany())
			{
				Reason = LOCTEXT("YouCantTradeQuantity", "You don't have any space left to buy any of this resource (one resource type per cargo bay).");
			}
			else
			{
				Reason = LOCTEXT("CantTradeQuantity", "The buyer doesn't have any space left to buy any of this resource.");
			}
			return false;
		}
	}

	// No possible transaction
	else
	{
		return false;
	}

	return true;
}

int32 SFlareTradeMenu::GetMaxTransactionAmount() const
{
	int32 ResourcePrice = TransactionSourceSpacecraft->GetCurrentSector()->GetTransfertResourcePrice(TransactionSourceSpacecraft, TransactionDestinationSpacecraft, TransactionResource);
	int32 MaxAffordableQuantity = FMath::Max(int64(0), TransactionDestinationSpacecraft->GetCompany()->GetMoney()) / ResourcePrice;
	if(TransactionDestinationSpacecraft->GetCompany() == TransactionSourceSpacecraft->GetCompany() || MenuManager->GetGame()->GetQuestManager()->IsTradeQuestUseStation(TransactionDestinationSpacecraft))
	{
		MaxAffordableQuantity = INT_MAX;
	}

	int32 ResourceMaxQuantity = FMath::Min(TransactionSourceSpacecraft->GetActiveCargoBay()->GetResourceQuantity(TransactionResource, MenuManager->GetPC()->GetCompany()),
		TransactionDestinationSpacecraft->GetActiveCargoBay()->GetFreeSpaceForResource(TransactionResource, MenuManager->GetPC()->GetCompany()));

	return FMath::Min(MaxAffordableQuantity, ResourceMaxQuantity);
}

#undef LOCTEXT_NAMESPACE

