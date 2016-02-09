
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
	TargetFactory = InArgs._Factory;
	MenuManager = InArgs._MenuManager;
	ShipList.Empty();
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
					.Text(this, &SFlareFactoryInfo::GetFactoryName, TargetFactory)
				]

				// Factory description
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareFactoryInfo::GetFactoryDescription, TargetFactory)
				]

				// Factory production cycle description
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareFactoryInfo::GetFactoryCycleInfo, TargetFactory)
				]

				// Ship building
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SAssignNew(ShipSelector, SComboBox<UFlareSpacecraftCatalogEntry*>)
					.OptionsSource(&ShipList)
					.OnGenerateWidget(this, &SFlareFactoryInfo::OnGenerateShipComboLine)
					.OnSelectionChanged(this, &SFlareFactoryInfo::OnShipComboLineSelectionChanged)
					.ComboBoxStyle(&Theme.ComboBoxStyle)
					.ForegroundColor(FLinearColor::White)
					.Visibility(this, &SFlareFactoryInfo::GetShipSelectorVisibility, TargetFactory)
					[
						SNew(STextBlock)
						.Text(this, &SFlareFactoryInfo::OnGetCurrentShipComboLine)
						.TextStyle(&Theme.TextFont)
					]
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
								.Visibility(this, &SFlareFactoryInfo::GetStartProductionVisibility, TargetFactory)
								.OnClicked(this, &SFlareFactoryInfo::OnStartProduction, TargetFactory)
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
								.Visibility(this, &SFlareFactoryInfo::GetStopProductionVisibility, TargetFactory)
								.OnClicked(this, &SFlareFactoryInfo::OnStopProduction, TargetFactory)
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
									.Percent(this, &SFlareFactoryInfo::GetProductionProgress, TargetFactory)
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
					]
				]
			]
		]
	];

	// Init buildable ship list
	ShipList.Empty();
	if (TargetFactory && TargetFactory->HasCreateShipAction())
	{
		UFlareSpacecraftCatalogEntry* SelectedEntry = NULL;
		UFlareSpacecraftCatalog* SpacecraftCatalog = MenuManager->GetGame()->GetSpacecraftCatalog();

		for (int SpacecraftIndex = 0; SpacecraftIndex < SpacecraftCatalog->ShipCatalog.Num(); SpacecraftIndex++)
		{
			UFlareSpacecraftCatalogEntry* Entry = SpacecraftCatalog->ShipCatalog[SpacecraftIndex];
			FFlareSpacecraftDescription* Description = &Entry->Data;
			ShipList.Add(Entry);

			if (Description->Identifier == TargetFactory->GetTargetShipClass())
			{
				SelectedEntry = Entry;
			}
		}

		ShipSelector->SetSelectedItem(SelectedEntry);
	}

	// Update factory
	UpdateFactoryLimits();
}


void SFlareFactoryInfo::UpdateFactoryLimits()
{
	check(TargetFactory);
	LimitList->ClearChildren();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	UFlareSimulatedSpacecraft* SimulatedSpacecraft = TargetFactory->GetParent();

	// Iterate all output resources
	for (int ResourceIndex = 0; ResourceIndex < TargetFactory->GetDescription()->CycleCost.OutputResources.Num(); ResourceIndex++)
	{
		const FFlareFactoryResource* FactoryResource = &TargetFactory->GetDescription()->CycleCost.OutputResources[ResourceIndex];
		FFlareResourceDescription* Resource = &FactoryResource->Resource->Data;
		check(Resource);
				
		// Production resource limiter
		bool ResourceLimitEnabled = TargetFactory->HasOutputLimit(Resource);
		FText ProductionCycleStatusText;
		if (ResourceLimitEnabled)
		{
			if (TargetFactory->GetOutputLimit(Resource) == 0)
			{
				ProductionCycleStatusText = FText::Format(LOCTEXT("ResourceLimitFormat", "{0} output is dumped"), Resource->Acronym);
			}
			else
			{
				ProductionCycleStatusText = FText::Format(LOCTEXT("ResourceLimitFormat", "{0} output limited to {1}"),
					Resource->Acronym,
					FText::AsNumber(TargetFactory->GetOutputLimit(Resource) * SimulatedSpacecraft->GetDescription()->CargoBayCapacity));
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
				.Visibility(this, &SFlareFactoryInfo::GetDecreaseOutputLimitVisibility, TargetFactory, Resource)
				.OnClicked(this, &SFlareFactoryInfo::OnDecreaseOutputLimit, TargetFactory, Resource)
				.Text(FText::FromString(TEXT("-")))
				.Transparent(true)
				.Width(1)
			]

			// Limit increase
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SFlareButton)
				.Visibility(this, &SFlareFactoryInfo::GetIncreaseOutputLimitVisibility, TargetFactory, Resource)
				.OnClicked(this, &SFlareFactoryInfo::OnIncreaseOutputLimit, TargetFactory, Resource)
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
		Result = FText::Format(LOCTEXT("FactoryNameFormat", "{0} - {1}"),
			FText::FromString(Factory->GetDescription()->Name.ToString().ToUpper()),
			Factory->GetFactoryStatus());
	}

	return Result;
}

FText SFlareFactoryInfo::GetFactoryDescription(UFlareFactory* Factory) const
{
	return Factory ? Factory->GetDescription()->Description : FText();
}

FText SFlareFactoryInfo::GetFactoryCycleInfo(UFlareFactory* Factory) const
{
	return Factory ? Factory->GetFactoryCycleInfo() : FText();
}

FText SFlareFactoryInfo::GetFactoryStatus(UFlareFactory* Factory) const
{
	return Factory ? Factory->GetFactoryStatus() : FText();
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
	Ship building
----------------------------------------------------*/

TSharedRef<SWidget> SFlareFactoryInfo::OnGenerateShipComboLine(UFlareSpacecraftCatalogEntry* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(STextBlock)
		.Text(Item->Data.Name)
		.TextStyle(&Theme.TextFont);
}

void SFlareFactoryInfo::OnShipComboLineSelectionChanged(UFlareSpacecraftCatalogEntry* Item, ESelectInfo::Type SelectInfo)
{
	if (Item && TargetFactory)
	{
		TargetFactory->SetTargetShipClass(Item->Data.Identifier);
	}
}

FText SFlareFactoryInfo::OnGetCurrentShipComboLine() const
{
	UFlareSpacecraftCatalogEntry* Item = ShipSelector->GetSelectedItem();
	return Item ? Item->Data.Name : LOCTEXT("Select", "Select a ship");
}

EVisibility SFlareFactoryInfo::GetShipSelectorVisibility(UFlareFactory* Factory) const
{
	return (Factory->HasCreateShipAction() ? EVisibility::Visible : EVisibility::Collapsed);
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
