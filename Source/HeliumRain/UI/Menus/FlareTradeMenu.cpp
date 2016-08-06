
#include "../../Flare.h"
#include "FlareTradeMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Game/FlareSectorHelper.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"
#include "../Components/FlareRoundButton.h"
#include "../Components/FlareCargoInfo.h"
#include "../../Economy/FlareCargoBay.h"

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
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SVerticalBox)

		// Content block
		+ SVerticalBox::Slot()
		.Padding(Theme.ContentPadding)
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

						// Current ship's cargo
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						.HAlign(HAlign_Left)
						[
							SAssignNew(LeftCargoBay, SHorizontalBox)
						]

						// Help text
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(LOCTEXT("HelpText", "Your ship is ready to trade with another spacecraft."))
						]

						// Resource prices title
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.TitlePadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.SubTitleFont)
							.Text(LOCTEXT("ResourcePrices", "Local resource prices"))
						]

						// Help text
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						.HAlign(HAlign_Fill)
						[
							SAssignNew(ResourcePriceList, SVerticalBox)
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
							SAssignNew(ShipList, SFlareShipList)
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
						]
				
						// Ship's cargo
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.ContentPadding)
						.HAlign(HAlign_Fill)
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SAssignNew(RightCargoBay, SHorizontalBox)
								.Visibility(this, &SFlareTradeMenu::GetTradingVisibility)
							]

							+ SHorizontalBox::Slot()
							.HAlign(HAlign_Right)
							.VAlign(VAlign_Center)
							[
								SNew(SFlareButton)
								.Text(LOCTEXT("BackToSelection", "Change target"))
								.Icon(FFlareStyleSet::GetIcon("Stop"))
								.OnClicked(this, &SFlareTradeMenu::OnBackToSelection)
								.Visibility(this, &SFlareTradeMenu::GetBackToSelectionVisibility)
								.Width(4)
							]
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
								]

								// Invalid transaction
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(Theme.ContentPadding)
								[
									SNew(STextBlock)
									.TextStyle(&Theme.TextFont)
									.Text(this, &SFlareTradeMenu::GetTransactionInvalidDetails)
									.Visibility(this, &SFlareTradeMenu::GetTransactionInvalidVisibility)
								]

								// Quantity
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(Theme.ContentPadding)
								[
									SAssignNew(QuantitySlider, SSlider)
									.Style(&Theme.SliderStyle)
									.Value(0)
									.OnValueChanged(this, &SFlareTradeMenu::OnResourceQuantityChanged)
									.Visibility(this, &SFlareTradeMenu::GetTransactionDetailsVisibility)
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
									.FullHide(true)
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

	// First-person trading override
	AFlareSpacecraft* PhysicalSpacecraft = TargetLeftSpacecraft->GetActive();
	if (TargetLeftSpacecraft->IsActive())
	{
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
	if (!PhysicalSpacecraft)
	{
		// Add stations
		for (int32 SpacecraftIndex = 0; SpacecraftIndex < ParentSector->GetSectorStations().Num(); SpacecraftIndex++)
		{
			UFlareSimulatedSpacecraft* StationCandidate = ParentSector->GetSectorStations()[SpacecraftIndex];
			if (StationCandidate && StationCandidate != LeftSpacecraft && StationCandidate != RightSpacecraft
			 && StationCandidate->GetDescription()->CargoBayCount > 0
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
			 && ShipCandidate->GetDescription()->CargoBayCount > 0
			 && ShipCandidate->GetDamageSystem()->IsAlive()
			 && ShipCandidate->GetCompany() == MenuManager->GetPC()->GetCompany())
			{
				ShipList->AddShip(ShipCandidate);
			}
		}
	}

	// Resource prices
	ResourcePriceList->ClearChildren();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	TArray<UFlareResourceCatalogEntry*> ResourceList = MenuManager->GetGame()->GetResourceCatalog()->GetResourceList();
	ResourceList.Sort(&SortByResourceType);

	// Resource prices
	for (int32 ResourceIndex = 0; ResourceIndex < ResourceList.Num(); ResourceIndex++)
	{
		FFlareResourceDescription& Resource = ResourceList[ResourceIndex]->Data;
		ResourcePriceList->AddSlot()
		.Padding(FMargin(1))
		[
			SNew(SBorder)
			.Padding(FMargin(1))
			.BorderImage((ResourceIndex % 2 == 0 ? &Theme.EvenBrush : &Theme.OddBrush))
			[
				SNew(SBox)
				.WidthOverride(Theme.ContentWidth)
				.Padding(FMargin(0))
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					// Icon
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBorder)
						.Padding(FMargin(0))
						.BorderImage(&Resource.Icon)
						[
							SNew(SBox)
							.WidthOverride(Theme.ResourceWidth)
							.HeightOverride(Theme.ResourceHeight)
							.Padding(FMargin(0))
							[
								SNew(STextBlock)
								.TextStyle(&Theme.TextFont)
								.Text(Resource.Acronym)
							]
						]
					]

					// Price
					+ SHorizontalBox::Slot()
					.Padding(Theme.ContentPadding)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.TextStyle(&Theme.TextFont)
						.Text(this, &SFlareTradeMenu::GetResourcePriceInfo, &Resource)
					]
				]
			]
		];
	}

	// Setup widgets
	AFlarePlayerController* PC = MenuManager->GetPC();
	FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftCargoBay);
	FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightCargoBay);
	ShipList->RefreshList();
	ShipList->SetVisibility(EVisibility::Visible);
}

void SFlareTradeMenu::FillTradeBlock(UFlareSimulatedSpacecraft* TargetSpacecraft, UFlareSimulatedSpacecraft* OtherSpacecraft, TSharedPtr<SHorizontalBox> CargoBay)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	CargoBay->ClearChildren();

	// Both spacecrafts are set
	if (TargetSpacecraft)
	{
		UFlareCargoBay* SpacecraftCargoBay = TargetSpacecraft->GetCargoBay();
		for (uint32 CargoIndex = 0; CargoIndex < SpacecraftCargoBay->GetSlotCount(); CargoIndex++)
		{
			FFlareCargo* Cargo = SpacecraftCargoBay->GetSlot(CargoIndex);
			CargoBay->AddSlot()
			[
				SNew(SFlareCargoInfo)
				.Spacecraft(TargetSpacecraft)
				.CargoIndex(CargoIndex)
				.OnClicked(this, &SFlareTradeMenu::OnTransferResources, TargetSpacecraft, OtherSpacecraft, Cargo->Resource)
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
	LeftCargoBay->ClearChildren();
	RightCargoBay->ClearChildren();

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


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

EVisibility SFlareTradeMenu::GetTradingVisibility() const
{
	return (TargetLeftSpacecraft && TargetRightSpacecraft) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlareTradeMenu::GetBackToSelectionVisibility() const
{
	if (IsEnabled() && TargetLeftSpacecraft->IsActive())
	{
		return EVisibility::Collapsed;
	}
	else
	{
		return GetTradingVisibility();
	}
}

EVisibility SFlareTradeMenu::GetTransactionDetailsVisibility() const
{
	return IsEnabled() && IsTransactionValid() ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlareTradeMenu::GetTransactionInvalidVisibility() const
{
	return (IsEnabled() && !IsTransactionValid() && TransactionSourceSpacecraft && TransactionDestinationSpacecraft && TransactionResource) ? EVisibility::Visible : EVisibility::Collapsed;
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
		return LOCTEXT("NoSelectedSpacecraft", "No spacecraft selected");
	}
}

FText SFlareTradeMenu::GetTransactionDetails() const
{
	if (TransactionSourceSpacecraft && TransactionDestinationSpacecraft && TransactionResource)
	{
		return FText::Format(LOCTEXT("TradeInfoFormat", "Trading {0}x {1} from {2} to {3}."),
			FText::AsNumber(TransactionQuantity),
			TransactionResource->Name,
			FText::FromName(TransactionSourceSpacecraft->GetImmatriculation()),
			FText::FromName(TransactionDestinationSpacecraft->GetImmatriculation()));
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
		return FText::Format(LOCTEXT("TradeInvalidInfoFormat", "Trading of {0} is unauthorized from {1} to {2}"),
			TransactionResource->Name,
			FText::FromName(TransactionSourceSpacecraft->GetImmatriculation()),
			FText::FromName(TransactionDestinationSpacecraft->GetImmatriculation()));
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

			VariationText = FText::Format(LOCTEXT("ResourceVariationFormat", " ({0}{1}%)"),
							(Variation > 0 ?
								 FText::FText(LOCTEXT("ResourceVariationFormatSignPlus","+")) :
								 FText::FText(LOCTEXT("ResourceVariationFormatSignMinus","-"))),
						  FText::AsNumber(FMath::Abs(Variation) * 100.0f, &MoneyFormat));
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
	UFlareSimulatedSpacecraft* Spacecraft = SpacecraftContainer->ShipInterfacePtr;

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
		FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightCargoBay);
		FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftCargoBay);
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
		TransactionQuantity = FMath::Min(TransactionSourceSpacecraft->GetCargoBay()->GetResourceQuantity(TransactionResource),
		                            TransactionDestinationSpacecraft->GetCargoBay()->GetFreeSpaceForResource(TransactionResource));
		QuantitySlider->SetValue(1.0f);

		UpdatePrice();
	}
}

void SFlareTradeMenu::OnResourceQuantityChanged(float Value)
{
	// Force slider value, update quantity
	int32 ResourceMaxQuantity = FMath::Min(TransactionSourceSpacecraft->GetCargoBay()->GetResourceQuantity(TransactionResource),
		                              TransactionDestinationSpacecraft->GetCargoBay()->GetFreeSpaceForResource(TransactionResource));

	TransactionQuantity = FMath::Lerp((int32)1, ResourceMaxQuantity, Value);

	if(ResourceMaxQuantity >= 1000 && (TransactionQuantity - ResourceMaxQuantity) > 50)
	{
		TransactionQuantity = (TransactionQuantity / 50) * 50;
	}
	else if(ResourceMaxQuantity >= 100 && (TransactionQuantity - ResourceMaxQuantity) > 10)
	{
		TransactionQuantity = (TransactionQuantity / 10) * 10;
	}

	if (ResourceMaxQuantity == 1)
	{
		QuantitySlider->SetValue(1.0f);
	}
	else
	{
		QuantitySlider->SetValue((float)(TransactionQuantity - 1) / (float)(ResourceMaxQuantity - 1));
	}

	UpdatePrice();
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
	FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightCargoBay);
	FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftCargoBay);
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
	FillTradeBlock(TargetRightSpacecraft, TargetLeftSpacecraft, RightCargoBay);
	FillTradeBlock(TargetLeftSpacecraft, TargetRightSpacecraft, LeftCargoBay);
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
	RightCargoBay->ClearChildren();
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

	if(IsTransactionValid())
	{
		if (TransactionSourceSpacecraft && TransactionDestinationSpacecraft->GetCompany() != TransactionSourceSpacecraft->GetCompany() && ResourceUnitPrice > 0)
		{
			int64 TransactionPrice = TransactionQuantity * ResourceUnitPrice;
			bool AllowDepts = (TransactionDestinationSpacecraft->GetResourceUseType(TransactionResource) == EFlareResourcePriceContext::ConsumerConsumption);
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

bool SFlareTradeMenu::IsTransactionValid() const
{
	if (TransactionSourceSpacecraft && TransactionDestinationSpacecraft && TransactionResource)
	{
		if(TransactionSourceSpacecraft->GetCompany() == TransactionDestinationSpacecraft->GetCompany())
		{
			return true;
		}
		if (TransactionSourceSpacecraft->GetCargoBay()->WantSell(TransactionResource) && TransactionDestinationSpacecraft->GetCargoBay()->WantBuy(TransactionResource))
		{
			return true;
		}
	}
	return false;
}

#undef LOCTEXT_NAMESPACE

