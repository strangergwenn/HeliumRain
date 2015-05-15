
#include "../../Flare.h"
#include "FlarePartInfo.h"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlarePartInfo::Construct(const FArguments& InArgs)
{
	ShowOwnershipInfo = InArgs._ShowOwnershipInfo;

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)

		// Part icon
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SImage).Image(&InArgs._Description->MeshPreviewBrush)
		]

		// Content box
		+ SHorizontalBox::Slot()
		[
			SAssignNew(Details, SVerticalBox)

			// Title and cost
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			.Padding(FMargin(10))
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// Title
				+ SHorizontalBox::Slot()
				[
					SNew(STextBlock)
					.Text(InArgs._Description->Name)
					.TextStyle(FFlareStyleSet::Get(), "Flare.Title3")
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
					.TextStyle(FFlareStyleSet::Get(), "Flare.Text")
				]
			]

			// Characteristics
			+ SVerticalBox::Slot()
			.Padding(FMargin(10, 2, 10, 2))
			.AutoHeight()
			[
				SAssignNew(InfoBox, SHorizontalBox)
			]
		]
	];

	BuildInfoBlock(InfoBox, InArgs._Description, false);

	// Toggle cost
	if (InArgs._Description)
	{
		PartCost = InArgs._Description->Cost;
	}
	SetOwned(InArgs._IsOwned);
}


/*----------------------------------------------------
	Content
----------------------------------------------------*/

void SFlarePartInfo::SetOwned(bool State)
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
			CostLabel->SetText(FString::FromInt(PartCost));
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

void SFlarePartInfo::BuildInfoBlock(TSharedPtr<SHorizontalBox>& Box, const FFlareShipComponentDescription* Desc, bool ShowHelpers)
{
	Box->ClearChildren();

	// Armor
	AddCharacteristicToBlock(Box,
		"Armor",
		FString::FromInt(Desc->ArmorHitPoints + Desc->HitPoints),
		FFlareStyleSet::GetIcon("Armor"),
		ShowHelpers);

	// Gun
	if(Desc->GunCharacteristics.IsGun)
	{
		AddCharacteristicToBlock(Box,
			"Power",
			FString::FromInt(Desc->GunCharacteristics.AmmoPower) + " kJ",
			FFlareStyleSet::GetIcon("Shell"),
			ShowHelpers);
		AddCharacteristicToBlock(Box,
			"Rate of fire",
			FString::FromInt(Desc->GunCharacteristics.AmmoRate) + " rpm",
			FFlareStyleSet::GetIcon("Rate"),
			ShowHelpers);
		AddCharacteristicToBlock(Box,
			"Magazine",
			FString::FromInt(Desc->GunCharacteristics.AmmoCapacity),
			FFlareStyleSet::GetIcon("Ammo"),
			ShowHelpers);
	}

	if(Desc->EngineCharacteristics.IsEngine)
	{
		if(Desc->EngineCharacteristics.AngularAccelerationRate > 0)
		{
			AddCharacteristicToBlock(Box,
				"Turn rating",
				FString::FromInt(Desc->EngineCharacteristics.AngularAccelerationRate),
				FFlareStyleSet::GetIcon("RCS"),
				ShowHelpers);
		}
		AddCharacteristicToBlock(Box,
			"Thrust",
			FString::FromInt(Desc->EngineCharacteristics.EnginePower) + " kN",
			FFlareStyleSet::GetIcon("Propulsion"),
			ShowHelpers);
	}
}

void SFlarePartInfo::AddCharacteristicToBlock(TSharedPtr<SHorizontalBox>& Box, FString Label, FString Value, const FSlateBrush* Icon, bool ShowHelpers)
{
	TSharedPtr<SVerticalBox> TempBox;

	Box->AddSlot().HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(SImage).Image(Icon)
			]

			// Text
			+ SHorizontalBox::Slot()
			.Padding(FMargin(5, 0))
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SAssignNew(TempBox, SVerticalBox)

				// Value
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Top)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(Value))
					.TextStyle(FFlareStyleSet::Get(), "Flare.Title3")
				]
			]
		];

	// Helpers
	if (ShowHelpers)
	{
		TempBox->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(Label))
				.TextStyle(FFlareStyleSet::Get(), "Flare.SmallText")
			];
	}
}
