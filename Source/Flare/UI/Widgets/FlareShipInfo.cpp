
#include "../../Flare.h"
#include "FlareShipInfo.h"
#include "../../Spacecrafts/FlareSpacecraftInterface.h"
#include "../../Player/FlarePlayerController.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareShipInfo::Construct(const FArguments& InArgs)
{
	ShowOwnershipInfo = InArgs._ShowOwnershipInfo;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	const FFlareSpacecraftSave* CurrentShipData = InArgs._Ship->Save();
	const FFlareSpacecraftDescription* Description = InArgs._Player->GetGame()->GetSpacecraftCatalog()->Get(CurrentShipData->Identifier);

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)

		// Ship icon
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SImage).Image(&Description->MeshPreviewBrush)
		]

		// Content box
		+ SHorizontalBox::Slot()
		[
			SAssignNew(Details, SVerticalBox)

			// Title and cost
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			.Padding(FMargin(10, 2))
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// Title
				+ SHorizontalBox::Slot()
				[
					SNew(STextBlock)
					.Text(FText::FromString(InArgs._Ship->GetImmatriculation()))
					.TextStyle(&Theme.SubTitleFont)
				]

				// Cost icon
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SAssignNew(CostImage, SImage)
				]

				// Cost label
				+ SHorizontalBox::Slot()
				.Padding(10, 0)
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SAssignNew(CostLabel, STextBlock)
					.TextStyle(&Theme.TextFont)
				]
			]

			// Characteristics
			+ SVerticalBox::Slot()
			.Padding(FMargin(10, 2, 20, 2))
			.AutoHeight()
			[
				SAssignNew(InfoBox, SHorizontalBox)
			]
		]
	];
	
	// Toggle cost
	ShipCost = Description->Cost;
	SetOwned(InArgs._IsOwned);
}


/*----------------------------------------------------
	Content
----------------------------------------------------*/

void SFlareShipInfo::SetOwned(bool State)
{
	IsOwned = State;
	if (ShowOwnershipInfo)
	{
		if (IsOwned)
		{
			CostLabel->SetVisibility(EVisibility::Collapsed);
			CostImage->SetImage(FFlareStyleSet::GetIcon("Owned"));
		}
		else
		{
			CostLabel->SetText(FString::FromInt(ShipCost));
			CostLabel->SetVisibility(EVisibility::Visible);
			CostImage->SetImage(FFlareStyleSet::GetIcon("Cost"));
		}
	}
	else
	{
		CostLabel->SetVisibility(EVisibility::Collapsed);
		CostImage->SetVisibility(EVisibility::Collapsed);
	}
}

/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

