
#include "../../Flare.h"
#include "FlareFactoryInfo.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Economy/FlareFactory.h"

#define LOCTEXT_NAMESPACE "FlareFactoryInfo"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareFactoryInfo::Construct(const FArguments& InArgs)
{
	Factory = InArgs._Factory;
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	// Structure
	ChildSlot
	.Padding(Theme.ContentPadding)
	[
		SNew(SBorder)
		.BorderImage(&Theme.ListActiveBackground)
		[
			SNew(SHorizontalBox)

			// Decorator
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SImage)
				.Image(&Theme.ButtonActiveDecorator)
			]

			// Factory data
			+ SHorizontalBox::Slot()
			[
				SNew(SVerticalBox)

				// Factory name
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.NameFont)
					.Text(this, &SFlareFactoryInfo::GetFactoryName, Factory)
				]

				// Factory description
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareFactoryInfo::GetFactoryDescription, Factory)
				]

				// Factory production cycle description
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareFactoryInfo::GetFactoryCycleInfo, Factory)
				]
			
				// Factory production status
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(SHorizontalBox)
					
					// Factory status (left pane)
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(SVerticalBox)

						// Progress box
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)

							// Factory start production button
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Visibility(this, &SFlareFactoryInfo::GetStartProductionVisibility, Factory)
								.OnClicked(this, &SFlareFactoryInfo::OnStartProduction, Factory)
								.Text(FText())
								.HelpText(LOCTEXT("StartProduction", "Start production"))
								.Icon(FFlareStyleSet::GetIcon("Load"))
								.Transparent(true)
								.Width(1)
							]

							// Factory stop production button
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SFlareButton)
								.Visibility(this, &SFlareFactoryInfo::GetStopProductionVisibility, Factory)
								.OnClicked(this, &SFlareFactoryInfo::OnStopProduction, Factory)
								.Text(FText())
								.HelpText(LOCTEXT("StopProduction", "Stop production"))
								.Icon(FFlareStyleSet::GetIcon("Stop"))
								.Transparent(true)
								.Width(1)
							]

							// Factory progress
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.ContentPadding)
							[
								SNew(SBox)
								.VAlign(VAlign_Center)
								.HAlign(HAlign_Fill)
								.WidthOverride(Theme.ContentWidth / 3)
								[
									SNew(SProgressBar)
									.Percent(this, &SFlareFactoryInfo::GetProductionProgress, Factory)
									.Style(&Theme.ProgressBarStyle)
								]
							]
						]
					]

					// Factory limits (right pane)
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Right)
					[
						SAssignNew(LimitList, SVerticalBox)

						//// Factory mode control
						//+ SVerticalBox::Slot()
						//.AutoHeight()
						//.Padding(Theme.SmallContentPadding)
						//[
						//	SNew(SHorizontalBox)

						//	// Factory switch mode
						//	+ SHorizontalBox::Slot()
						//	.AutoWidth()
						//	[
						//		SNew(SFlareButton)
						//		.OnClicked(this, &SFlareFactoryInfo::OnSwitchProductionCyclesLimit, Factory)
						//		.Text(ProductionCycleStatusText)
						//		.Width(ProductionLimitEnabled ? 6 : 8)
						//	]

						//	// Factory remove production cycle button
						//	+ SHorizontalBox::Slot()
						//	.AutoWidth()
						//	[
						//		SNew(SFlareButton)
						//		.Visibility(this, &SFlareFactoryInfo::GetProductionCyclesLimitVisibility, Factory)
						//		.OnClicked(this, &SFlareFactoryInfo::OnDecreaseProductionCycles, Factory)
						//		.Text(FText::FromString(TEXT("-")))
						//		.Width(1)
						//	]

						//	// Factory add production cycle button
						//	+ SHorizontalBox::Slot()
						//	.AutoWidth()
						//	[
						//		SNew(SFlareButton)
						//		.Visibility(this, &SFlareFactoryInfo::GetProductionCyclesLimitVisibility, Factory)
						//		.OnClicked(this, &SFlareFactoryInfo::OnIncreaseProductionCycles, Factory)
						//		.Text(FText::FromString(TEXT("+")))
						//		.Width(1)
						//	]
						//]
					]
				]
			]
		]
	];
}


void SFlareFactoryInfo::UpdateFactoryLimits()
{
	check(Factory);
	LimitList->ClearChildren();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	UFlareSimulatedSpacecraft* SimulatedSpacecraft = Factory->GetParent();

	// Production cycle limiter
	/*bool ProductionLimitEnabled = !Factory->HasInfiniteCycle();
	FText ProductionCycleStatusText;
	if (ProductionLimitEnabled)
	{
	ProductionCycleStatusText = FText::Format(LOCTEXT("ProductionLimitFormat", "Limited to {0} cycles (clear)"),
	FText::AsNumber(Factory->GetCycleCount()));
	}
	else
	{
	ProductionCycleStatusText = LOCTEXT("ProductionLimitEnable", "Limit production cycles");
	}*/

	// Iterate all output resources
	for (int ResourceIndex = 0; ResourceIndex < Factory->GetDescription()->OutputResources.Num(); ResourceIndex++)
	{
		const FFlareFactoryResource* FactoryResource = &Factory->GetDescription()->OutputResources[ResourceIndex];
		FFlareResourceDescription* Resource = &FactoryResource->Resource->Data;
		check(Resource);
				
		// Production resource limiter
		bool ResourceLimitEnabled = Factory->HasOutputLimit(Resource);
		FText ProductionCycleStatusText;
		if (ResourceLimitEnabled)
		{
			if (Factory->GetOutputLimit(Resource) == 0)
			{
				ProductionCycleStatusText = FText::Format(LOCTEXT("ResourceLimitFormat", "{0} output is dumped"), Resource->Acronym);
			}
			else
			{
				ProductionCycleStatusText = FText::Format(LOCTEXT("ResourceLimitFormat", "{0} output limited to {1}"),
					Resource->Acronym,
					FText::AsNumber(Factory->GetOutputLimit(Resource) * SimulatedSpacecraft->GetDescription()->CargoBayCapacity));
			}
		}
		else
		{
			ProductionCycleStatusText = FText::Format(LOCTEXT("ResourceLimitEnableFormat", "{0} output is unlimited"), Resource->Acronym);
		}

		// Add a new limiter slot
		LimitList->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		[
			SNew(SHorizontalBox)

			// Limit info
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(Theme.ContentPadding)
			[
				SNew(SBox)
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(ProductionCycleStatusText)
				]
			]

			// Limit decrease
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SFlareButton)
				.Visibility(this, &SFlareFactoryInfo::GetDecreaseOutputLimitVisibility, Factory, Resource)
				.OnClicked(this, &SFlareFactoryInfo::OnDecreaseOutputLimit, Factory, Resource)
				.Text(FText::FromString(TEXT("-")))
				.Transparent(true)
				.Width(1)
			]

			// Limit increase
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SFlareButton)
				.Visibility(this, &SFlareFactoryInfo::GetIncreaseOutputLimitVisibility, Factory, Resource)
				.OnClicked(this, &SFlareFactoryInfo::OnIncreaseOutputLimit, Factory, Resource)
				.Text(FText::FromString(TEXT("+")))
				.Transparent(true)
				.Width(1)
			]
		];
	}
}


/*----------------------------------------------------
	Content callbacks
----------------------------------------------------*/

FText SFlareFactoryInfo::GetFactoryName(UFlareFactory* Factory) const
{
	FText Result;

	if (Factory)
	{
		Result = FText::Format(LOCTEXT("FactoryNameFormat", "{0} ({1})"),
			FText::FromString(Factory->GetDescription()->Name.ToString().ToUpper()),
			GetFactoryStatus(Factory));
	}

	return Result;
}

FText SFlareFactoryInfo::GetFactoryDescription(UFlareFactory* Factory) const
{
	FText Result;

	if (Factory)
	{
		Result = Factory->GetDescription()->Description;
	}

	return Result;
}

FText SFlareFactoryInfo::GetFactoryCycleInfo(UFlareFactory* Factory) const
{
	FText CommaTextReference = LOCTEXT("Comma", " + ");
	FText ProductionCostText;
	FText ProductionOutputText;

	// Cycle cost in credits
	uint32 CycleCost = Factory->GetDescription()->ProductionCost;
	if (CycleCost > 0)
	{
		ProductionCostText = FText::Format(LOCTEXT("ProductionCostFormat", "{0} credits"), FText::AsNumber(CycleCost));
	}

	// Cycle cost in resources
	for (int ResourceIndex = 0; ResourceIndex < Factory->GetDescription()->InputResources.Num(); ResourceIndex++)
	{
		FText CommaText = ProductionCostText.IsEmpty() ? FText() : CommaTextReference;
		const FFlareFactoryResource* FactoryResource = &Factory->GetDescription()->InputResources[ResourceIndex];
		check(FactoryResource);

		ProductionCostText = FText::Format(LOCTEXT("ProductionResourcesFormat", "{0}{1} {2} {3}"),
			ProductionCostText, CommaText, FText::AsNumber(FactoryResource->Quantity), FactoryResource->Resource->Data.Acronym);
	}

	// Cycle output in factory actions
	for (int ActionIndex = 0; ActionIndex < Factory->GetDescription()->OutputActions.Num(); ActionIndex++)
	{
		FText CommaText = ProductionOutputText.IsEmpty() ? FText() : CommaTextReference;
		const FFlareFactoryAction* FactoryAction = &Factory->GetDescription()->OutputActions[ActionIndex];
		check(FactoryAction);

		switch (FactoryAction->Action)
		{
			// Ship production
		case EFlareFactoryAction::CreateShip:
			ProductionOutputText = FText::Format(LOCTEXT("ProductionActionsFormat", "{0}{1} {2} {3}"),
				ProductionOutputText, CommaText, FText::AsNumber(FactoryAction->Quantity),
				MenuManager->GetGame()->GetSpacecraftCatalog()->Get(FactoryAction->Identifier)->Name);
			break;

			// TODO
		case EFlareFactoryAction::DiscoverSector:
		case EFlareFactoryAction::GainTechnology:
		default:
			FLOGV("SFlareShipMenu::UpdateFactoryLimitsList : Unimplemented factory action %d", (FactoryAction->Action + 0));
		}
	}

	// Cycle output in resources
	for (int ResourceIndex = 0; ResourceIndex < Factory->GetDescription()->OutputResources.Num(); ResourceIndex++)
	{
		FText CommaText = ProductionOutputText.IsEmpty() ? FText() : CommaTextReference;
		const FFlareFactoryResource* FactoryResource = &Factory->GetDescription()->OutputResources[ResourceIndex];
		check(FactoryResource);

		ProductionOutputText = FText::Format(LOCTEXT("ProductionOutputFormat", "{0}{1} {2} {3}"),
			ProductionOutputText, CommaText, FText::AsNumber(FactoryResource->Quantity), FactoryResource->Resource->Data.Acronym);
	}

	return FText::Format(LOCTEXT("FactoryCycleInfoFormat", "Production cycle : {0} \u2192 {1} each {2}"),
		ProductionCostText, ProductionOutputText,
		FText::FromString(*UFlareGameTools::FormatTime(Factory->GetDescription()->ProductionTime, 2))); // FString needed here
}

FText SFlareFactoryInfo::GetFactoryStatus(UFlareFactory* Factory) const
{
	FText ProductionStatusText;

	if (Factory && Factory->IsActive())
	{
		if (!Factory->IsNeedProduction())
		{
			ProductionStatusText = LOCTEXT("ProductionNotNeeded", "Production not needed");
		}
		else if (Factory->HasCostReserved())
		{
			ProductionStatusText = FText::Format(LOCTEXT("ProductionInProgressFormat", "Producing ({0}{1})"),
				FText::FromString(*UFlareGameTools::FormatTime(Factory->GetRemainingProductionDuration(), 2)), // FString needed here
				Factory->HasOutputFreeSpace() ? FText() : LOCTEXT("ProductionNoSpace", ", not enough space"));
		}
		else if (Factory->HasInputMoney() && Factory->HasInputResources())
		{
			ProductionStatusText = LOCTEXT("ProductionWillStart", "Starting");
		}
		else
		{
			if (!Factory->HasInputMoney())
			{
				ProductionStatusText = LOCTEXT("ProductionNotEnoughMoney", "Waiting for credits");
			}

			if (!Factory->HasInputResources())
			{
				ProductionStatusText = LOCTEXT("ProductionNotEnoughResources", "Waiting for resources");
			}
		}
	}
	else if (Factory->IsPaused())
	{
		ProductionStatusText = FText::Format(LOCTEXT("ProductionPaused", "Paused ({0} to completion)"),
			FText::FromString(*UFlareGameTools::FormatTime(Factory->GetRemainingProductionDuration(), 2))); // FString needed here
	}
	else
	{
		ProductionStatusText = LOCTEXT("ProductionStopped", "Stopped");
	}

	return ProductionStatusText;
}

TOptional<float> SFlareFactoryInfo::GetProductionProgress(UFlareFactory* Factory) const
{
	if (Factory)
	{
		return ((float)Factory->GetProductedDuration() / (float)Factory->GetRemainingProductionDuration());
	}
	else
	{
		return 0;
	}
}

EVisibility SFlareFactoryInfo::GetStartProductionVisibility(UFlareFactory* Factory) const
{
	if (Factory)
	{
		return (!Factory->IsActive() ? EVisibility::Visible : EVisibility::Collapsed);
	}
	else
	{
		return EVisibility::Collapsed;
	}
}

EVisibility SFlareFactoryInfo::GetStopProductionVisibility(UFlareFactory* Factory) const
{
	if (Factory)
	{
		return (Factory->IsActive() ? EVisibility::Visible : EVisibility::Collapsed);
	}
	else
	{
		return EVisibility::Collapsed;
	}
}
//
//EVisibility SFlareFactoryInfo::GetProductionCyclesLimitVisibility(UFlareFactory* Factory) const
//{
//	return (!Factory->HasInfiniteCycle() ? EVisibility::Visible : EVisibility::Collapsed);
//}

EVisibility SFlareFactoryInfo::GetIncreaseOutputLimitVisibility(UFlareFactory* Factory, FFlareResourceDescription* Resource) const
{
	if (Factory)
	{
		uint32 MaxOutput = Factory->GetParent()->GetDescription()->CargoBayCount;
		return (Factory->HasOutputLimit(Resource) && Factory->GetOutputLimit(Resource) < MaxOutput ? EVisibility::Visible : EVisibility::Hidden);
	}
	else
	{
		return EVisibility::Hidden;
	}
}

EVisibility SFlareFactoryInfo::GetDecreaseOutputLimitVisibility(UFlareFactory* Factory, FFlareResourceDescription* Resource) const
{
	if (Factory)
	{
		return (!Factory->HasOutputLimit(Resource) || Factory->GetOutputLimit(Resource) > 0 ? EVisibility::Visible : EVisibility::Hidden);
	}
	else
	{
		return EVisibility::Hidden;
	}
}


/*----------------------------------------------------
	Action callbacks
----------------------------------------------------*/
	
void SFlareFactoryInfo::OnStartProduction(UFlareFactory* Factory)
{
	check(Factory);
	Factory->Start();
	UpdateFactoryLimits();
}

void SFlareFactoryInfo::OnStopProduction(UFlareFactory* Factory)
{
	check(Factory);
	Factory->Stop();
	UpdateFactoryLimits();
}
//
//void SFlareFactoryInfo::OnSwitchProductionCyclesLimit(UFlareFactory* Factory)
//{
//	Factory->SetInfiniteCycle(!Factory->HasInfiniteCycle());
//	UpdateFactoryLimitsList();
//}
//
//void SFlareFactoryInfo::OnIncreaseProductionCycles(UFlareFactory* Factory)
//{
//	Factory->SetCycleCount(Factory->GetCycleCount() + 1);
//	UpdateFactoryLimitsList();
//}
//
//void SFlareFactoryInfo::OnDecreaseProductionCycles(UFlareFactory* Factory)
//{
//	if (Factory->GetCycleCount() > 1)
//	{
//		Factory->SetCycleCount(Factory->GetCycleCount() - 1);
//		UpdateFactoryLimitsList();
//	}
//}

void SFlareFactoryInfo::OnDecreaseOutputLimit(UFlareFactory* Factory, FFlareResourceDescription* Resource)
{
	check(Factory);
	uint32 MaxOutput = Factory->GetParent()->GetDescription()->CargoBayCount;

	if (!Factory->HasOutputLimit(Resource))
	{
		Factory->SetOutputLimit(Resource, MaxOutput - 1);
	}
	else if (Factory->GetOutputLimit(Resource) > 0)
	{
		Factory->SetOutputLimit(Resource, Factory->GetOutputLimit(Resource) - 1);
	}
	UpdateFactoryLimits();
}

void SFlareFactoryInfo::OnIncreaseOutputLimit(UFlareFactory* Factory, FFlareResourceDescription* Resource)
{
	check(Factory);
	uint32 MaxOutput = Factory->GetParent()->GetDescription()->CargoBayCount;

	if (Factory->GetOutputLimit(Resource) + 1 >= MaxOutput)
	{
		Factory->ClearOutputLimit(Resource);
	}
	else
	{
		Factory->SetOutputLimit(Resource, Factory->GetOutputLimit(Resource) + 1);
	}
	UpdateFactoryLimits();
}

#undef LOCTEXT_NAMESPACE
