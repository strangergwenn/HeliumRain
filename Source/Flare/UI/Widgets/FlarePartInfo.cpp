
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

void SFlarePartInfo::BuildInfoBlock(TSharedPtr<SHorizontalBox>& Box, const FFlareShipModuleDescription* Desc, bool ShowHelpers)
{
	Box->ClearChildren();

	// Fill in the characteristics structure for everything except the cost
	for (int32 i = 0; i < Desc->Characteristics.Num(); i++)
	{
		TSharedPtr<SVerticalBox> TempBox;
		const FFlarePartCharacteristic& Characteristic = Desc->Characteristics[i];

		Box->AddSlot().HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(SImage).Image(GetCharacteristicBrush(Characteristic))
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
					.Text(GetCharacteristicInfo(Characteristic))
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
				.Text(GetCharacteristicLabel(Characteristic.CharacteristicType))
				.TextStyle(FFlareStyleSet::Get(), "Flare.SmallText")
			];
		}
	}
}

FString SFlarePartInfo::GetCharacteristicInfo(const FFlareShipModuleDescription* Desc, EFlarePartAttributeType::Type Type)
{
	for (int32 i = 0; i < Desc->Characteristics.Num(); i++)
	{
		const FFlarePartCharacteristic& Characteristic = Desc->Characteristics[i];

		if (Characteristic.CharacteristicType == Type)
		{
			return GetCharacteristicInfo(Characteristic);
		}
	}

	return "";
}

FString SFlarePartInfo::GetCharacteristicInfo(const FFlarePartCharacteristic& Characteristic)
{
	FString Unit;

	switch (Characteristic.CharacteristicType)
	{
		case EFlarePartAttributeType::AmmoRate:
			Unit += "rpm";
			break;
		case EFlarePartAttributeType::EnginePower:
			Unit += "kN";
			break;
		case EFlarePartAttributeType::Armor:
		case EFlarePartAttributeType::AmmoPower:
		case EFlarePartAttributeType::AmmoCapacity:
		case EFlarePartAttributeType::EngineTankDrain:
		case EFlarePartAttributeType::RCSAccelerationRating:
		default: break;
	}

	return FString::FromInt(Characteristic.CharacteristicValue) + " " + Unit;
}

FString SFlarePartInfo::GetCharacteristicLabel(EFlarePartAttributeType::Type Type)
{
	FString Label;

	switch (Type)
	{
		case EFlarePartAttributeType::Armor:
			Label = "Hull";
			break;
		case EFlarePartAttributeType::AmmoPower:
			Label = "Power";
			break;
		case EFlarePartAttributeType::AmmoRange:
			Label = "Range";
			break;
		case EFlarePartAttributeType::AmmoRate:
			Label += "Rate of fire";
			break;
		case EFlarePartAttributeType::AmmoCapacity:
			Label = "Magazine";
			break;
		case EFlarePartAttributeType::EngineTankDrain:
			Label = "Consumption";
			break;
		case EFlarePartAttributeType::RCSAccelerationRating:
			Label = "Turn rating";
			break;
		case EFlarePartAttributeType::EnginePower:
			Label = "Thrust";
			break;

		default: break;
	}

	return Label;
}

const FSlateBrush* SFlarePartInfo::GetCharacteristicBrush(const FFlarePartCharacteristic& Characteristic)
{
	const FSlateBrush* Result = NULL;

	switch (Characteristic.CharacteristicType)
	{
		case EFlarePartAttributeType::Armor:
			Result = FFlareStyleSet::GetIcon("Armor");
			break;
		case EFlarePartAttributeType::AmmoPower:
			Result = FFlareStyleSet::GetIcon("Shell");
			break;
		case EFlarePartAttributeType::AmmoRange:
			Result = FFlareStyleSet::GetIcon("Range");
			break;
		case EFlarePartAttributeType::AmmoRate:
			Result = FFlareStyleSet::GetIcon("Rate");
			break;
		case EFlarePartAttributeType::AmmoCapacity:
			Result = FFlareStyleSet::GetIcon("Ammo");
			break;
		case EFlarePartAttributeType::EnginePower:
			Result = FFlareStyleSet::GetIcon("ExhaustPower");
			break;
		case EFlarePartAttributeType::RCSAccelerationRating:
			Result = FFlareStyleSet::GetIcon("RCSPower");
			break;
		case EFlarePartAttributeType::EngineTankDrain:
			Result = FFlareStyleSet::GetIcon("Tank");
			break;

		default: break;
	}

	return Result;
}
