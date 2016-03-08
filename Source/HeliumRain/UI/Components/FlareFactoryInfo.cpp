
#include "../../Flare.h"
#include "FlareFactoryInfo.h"
#include "../../Game/FlareGame.h"
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
					.Text(this, &SFlareFactoryInfo::GetFactoryName)
				]

				// Factory description
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareFactoryInfo::GetFactoryDescription)
				]

				// Factory production cycle description
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareFactoryInfo::GetFactoryCycleInfo)
				]

				// Ship building
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Left)
				[
					SNew(SFlareButton)
					.Width(8)
					.Text(this, &SFlareFactoryInfo::GetTargetShipClassText)
					.HelpText(LOCTEXT("ChooseClass", "Pick a ship class to build, or change the current selection"))
					.OnClicked(this, &SFlareFactoryInfo::OnOpenSpacecraftOrder)
					.Visibility(this, &SFlareFactoryInfo::GetShipSelectorVisibility)
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
								.Visibility(this, &SFlareFactoryInfo::GetStartProductionVisibility)
								.OnClicked(this, &SFlareFactoryInfo::OnStartProduction)
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
								.Visibility(this, &SFlareFactoryInfo::GetStopProductionVisibility)
								.OnClicked(this, &SFlareFactoryInfo::OnStopProduction)
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
									.Percent(this, &SFlareFactoryInfo::GetProductionProgress)
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
				.Visibility(this, &SFlareFactoryInfo::GetDecreaseOutputLimitVisibility, Resource)
				.OnClicked(this, &SFlareFactoryInfo::OnDecreaseOutputLimit, Resource)
				.Text(FText::FromString(TEXT("-")))
				.Transparent(true)
				.Width(1)
			]

			// Limit increase
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SFlareButton)
				.Visibility(this, &SFlareFactoryInfo::GetIncreaseOutputLimitVisibility, Resource)
				.OnClicked(this, &SFlareFactoryInfo::OnIncreaseOutputLimit, Resource)
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

FText SFlareFactoryInfo::GetFactoryName() const
{
	FText Result;

	if (TargetFactory)
	{
		Result = FText::Format(LOCTEXT("FactoryNameFormat", "{0} - {1}"),
			FText::FromString(TargetFactory->GetDescription()->Name.ToString().ToUpper()),
			TargetFactory->GetFactoryStatus());
	}

	return Result;
}

FText SFlareFactoryInfo::GetFactoryDescription() const
{
	return TargetFactory ? TargetFactory->GetDescription()->Description : FText();
}

FText SFlareFactoryInfo::GetFactoryCycleInfo() const
{
	return TargetFactory ? TargetFactory->GetFactoryCycleInfo() : FText();
}

FText SFlareFactoryInfo::GetFactoryStatus() const
{
	return TargetFactory ? TargetFactory->GetFactoryStatus() : FText();
}

TOptional<float> SFlareFactoryInfo::GetProductionProgress() const
{
	if (TargetFactory)
	{
		return ((float)TargetFactory->GetProductedDuration() / (float)TargetFactory->GetCycleData().ProductionTime);
	}
	else
	{
		return 0;
	}
}

EVisibility SFlareFactoryInfo::GetStartProductionVisibility() const
{
	if (TargetFactory)
	{
		return (!TargetFactory->IsActive() ? EVisibility::Visible : EVisibility::Collapsed);
	}
	else
	{
		return EVisibility::Collapsed;
	}
}

EVisibility SFlareFactoryInfo::GetStopProductionVisibility() const
{
	if (TargetFactory)
	{
		return (TargetFactory->IsActive() ? EVisibility::Visible : EVisibility::Collapsed);
	}
	else
	{
		return EVisibility::Collapsed;
	}
}

EVisibility SFlareFactoryInfo::GetIncreaseOutputLimitVisibility(FFlareResourceDescription* Resource) const
{
	if (TargetFactory)
	{
		uint32 MaxOutput = TargetFactory->GetParent()->GetDescription()->CargoBayCount;
		return (TargetFactory->HasOutputLimit(Resource) && TargetFactory->GetOutputLimit(Resource) < MaxOutput ? EVisibility::Visible : EVisibility::Hidden);
	}
	else
	{
		return EVisibility::Hidden;
	}
}

EVisibility SFlareFactoryInfo::GetDecreaseOutputLimitVisibility(FFlareResourceDescription* Resource) const
{
	if (TargetFactory)
	{
		return (!TargetFactory->HasOutputLimit(Resource) || TargetFactory->GetOutputLimit(Resource) > 0 ? EVisibility::Visible : EVisibility::Hidden);
	}
	else
	{
		return EVisibility::Hidden;
	}
}

/*----------------------------------------------------
	Ship building
----------------------------------------------------*/

FText SFlareFactoryInfo::GetTargetShipClassText() const
{
	if (TargetFactory && TargetFactory->IsShipyard())
	{
		FName TargetShipClass = TargetFactory->GetTargetShipClass();
		if (TargetShipClass != NAME_None)
		{
			FFlareSpacecraftDescription* TargetShipDesc = MenuManager->GetGame()->GetSpacecraftCatalog()->Get(TargetShipClass);
			return FText::Format(LOCTEXT("CurrentShipFormat", "Building {0} (Change)"), TargetShipDesc->Name);
		}
		else
		{
			return LOCTEXT("ChooseClass", "Pick a ship class");
		}
	}
	else
	{
		return FText();
	}
}

EVisibility SFlareFactoryInfo::GetShipSelectorVisibility() const
{
	return (TargetFactory && TargetFactory->IsShipyard() ? EVisibility::Visible : EVisibility::Collapsed);
}


/*----------------------------------------------------
	Action callbacks
----------------------------------------------------*/

void SFlareFactoryInfo::OnOpenSpacecraftOrder()
{
	MenuManager->OpenSpacecraftOrder(TargetFactory);
}

void SFlareFactoryInfo::OnStartProduction()
{
	check(TargetFactory);
	TargetFactory->Start();
	UpdateFactoryLimits();
}

void SFlareFactoryInfo::OnStopProduction()
{
	check(TargetFactory);
	TargetFactory->Stop();
	UpdateFactoryLimits();
}

void SFlareFactoryInfo::OnDecreaseOutputLimit(FFlareResourceDescription* Resource)
{
	check(TargetFactory);
	uint32 MaxOutput = TargetFactory->GetParent()->GetDescription()->CargoBayCount;

	if (!TargetFactory->HasOutputLimit(Resource))
	{
		TargetFactory->SetOutputLimit(Resource, MaxOutput - 1);
	}
	else if (TargetFactory->GetOutputLimit(Resource) > 0)
	{
		TargetFactory->SetOutputLimit(Resource, TargetFactory->GetOutputLimit(Resource) - 1);
	}
	UpdateFactoryLimits();
}

void SFlareFactoryInfo::OnIncreaseOutputLimit(FFlareResourceDescription* Resource)
{
	check(TargetFactory);
	uint32 MaxOutput = TargetFactory->GetParent()->GetDescription()->CargoBayCount;

	if (TargetFactory->GetOutputLimit(Resource) + 1 >= MaxOutput)
	{
		TargetFactory->ClearOutputLimit(Resource);
	}
	else
	{
		TargetFactory->SetOutputLimit(Resource, TargetFactory->GetOutputLimit(Resource) + 1);
	}
	UpdateFactoryLimits();
}

#undef LOCTEXT_NAMESPACE
