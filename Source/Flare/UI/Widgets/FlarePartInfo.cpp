
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
	PartCost = InArgs._Description->Cost;
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

	// Fill in the characteristics structure for dynamic values
	for (int32 i = 0; i < Desc->Characteristics.Num(); i++)
	{
		const FFlareShipComponentCharacteristic& Characteristic = Desc->Characteristics[i];

		if (Characteristic.CharacteristicType <= EFlarePartCharacteristicType::RCSAccelerationRating)
		{
			AddCharacteristicToBlock(Box,
				GetCharacteristicLabel(Characteristic.CharacteristicType),
				GetCharacteristicInfo(Characteristic),
				GetCharacteristicBrush(Characteristic),
				ShowHelpers);
		}
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
					.Text(Value)
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
				.Text(Label)
				.TextStyle(FFlareStyleSet::Get(), "Flare.SmallText")
			];
	}
}

FString SFlarePartInfo::GetCharacteristicInfo(const FFlareShipComponentDescription* Desc, EFlarePartCharacteristicType::Type Type)
{
	for (int32 i = 0; i < Desc->Characteristics.Num(); i++)
	{
		const FFlareShipComponentCharacteristic& Characteristic = Desc->Characteristics[i];

		if (Characteristic.CharacteristicType == Type)
		{
			return GetCharacteristicInfo(Characteristic);
		}
	}

	return "";
}

FString SFlarePartInfo::GetCharacteristicInfo(const FFlareShipComponentCharacteristic& Characteristic)
{
	FString Unit;

	switch (Characteristic.CharacteristicType)
	{
		case EFlarePartCharacteristicType::AmmoRate:
			Unit += "rpm";
			break;
		case EFlarePartCharacteristicType::EnginePower:
			Unit += "kN";
			break;
		case EFlarePartCharacteristicType::AmmoPower:
		case EFlarePartCharacteristicType::AmmoCapacity:
		case EFlarePartCharacteristicType::EngineTankDrain:
		case EFlarePartCharacteristicType::RCSAccelerationRating:
		default: break;
	}

	return FString::FromInt(Characteristic.CharacteristicValue) + " " + Unit;
}

FString SFlarePartInfo::GetCharacteristicLabel(EFlarePartCharacteristicType::Type Type)
{
	FString Label;

	switch (Type)
	{
		case EFlarePartCharacteristicType::AmmoPower:
			Label = "Power";
			break;
		case EFlarePartCharacteristicType::AmmoRate:
			Label += "Rate of fire";
			break;
		case EFlarePartCharacteristicType::AmmoCapacity:
			Label = "Magazine";
			break;
		case EFlarePartCharacteristicType::EngineTankDrain:
			Label = "Consumption";
			break;
		case EFlarePartCharacteristicType::RCSAccelerationRating:
			Label = "Turn rating";
			break;
		case EFlarePartCharacteristicType::EnginePower:
			Label = "Thrust";
			break;

		default: break;
	}

	return Label;
}

const FSlateBrush* SFlarePartInfo::GetCharacteristicBrush(const FFlareShipComponentCharacteristic& Characteristic)
{
	const FSlateBrush* Result = NULL;

	switch (Characteristic.CharacteristicType)
	{
		case EFlarePartCharacteristicType::AmmoPower:
			Result = FFlareStyleSet::GetIcon("Shell");
			break;
		case EFlarePartCharacteristicType::AmmoRate:
			Result = FFlareStyleSet::GetIcon("Rate");
			break;
		case EFlarePartCharacteristicType::AmmoCapacity:
			Result = FFlareStyleSet::GetIcon("Ammo");
			break;
		case EFlarePartCharacteristicType::EnginePower:
			Result = FFlareStyleSet::GetIcon("Propulsion");
			break;
		case EFlarePartCharacteristicType::RCSAccelerationRating:
			Result = FFlareStyleSet::GetIcon("RCS");
			break;
		case EFlarePartCharacteristicType::EngineTankDrain:
			Result = FFlareStyleSet::GetIcon("Tank");
			break;

		default: break;
	}

	return Result;
}
