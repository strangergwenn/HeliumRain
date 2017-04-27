
#include "FlarePartInfo.h"
#include "../../Flare.h"
#include "../../Game/FlareGameTools.h"


#define LOCTEXT_NAMESPACE "FlareCompanyInfo"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlarePartInfo::Construct(const FArguments& InArgs)
{
	ShowOwnershipInfo = InArgs._ShowOwnershipInfo;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Create the layout
	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)

		// Part icon
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Top)
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
			.Padding(Theme.SmallContentPadding)
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// Title
				+ SHorizontalBox::Slot()
				[
					SNew(STextBlock)
					.Text(InArgs._Description->Name)
					.TextStyle(&Theme.NameFont)
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

	// Minimized
	if (InArgs._IsMinimized)
	{
		Details->SetVisibility(EVisibility::Collapsed);
	}
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
			CostLabel->SetText(FString::FromInt(UFlareGameTools::DisplayMoney(PartCost)));
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

void SFlarePartInfo::BuildInfoBlock(TSharedPtr<SHorizontalBox>& Box, const FFlareSpacecraftComponentDescription* Desc, bool ShowHelpers)
{
	Box->ClearChildren();

	// Armor
	AddCharacteristicToBlock(Box,
		LOCTEXT("Armor", "Armor"),
		FText::AsNumber(Desc->HitPoints),
		FFlareStyleSet::GetIcon("Armor"),
		ShowHelpers);

	// Gun
	if (Desc->WeaponCharacteristics.IsWeapon)
	{
		int32 Power = 0;
		switch(Desc->WeaponCharacteristics.DamageType)
		{
			case EFlareShellDamageType::ArmorPiercing:
				Power = Desc->WeaponCharacteristics.GunCharacteristics.KineticEnergy;
				break;
			case EFlareShellDamageType::HEAT:
				Power = Desc->WeaponCharacteristics.ExplosionPower;
				break;
		case EFlareShellDamageType::HighExplosive:
			Power = Desc->WeaponCharacteristics.AmmoFragmentCount * Desc->WeaponCharacteristics.ExplosionPower;
			break;
		}

		AddCharacteristicToBlock(Box,
			LOCTEXT("Power", "Power"),
			FText::Format(LOCTEXT("PowerInfoFormat", "{0} kJ"), FText::AsNumber(Power)),
			FFlareStyleSet::GetIcon("Shell"),
			ShowHelpers);

		if (Desc->WeaponCharacteristics.GunCharacteristics.IsGun)
		{
			AddCharacteristicToBlock(Box,
				LOCTEXT("RateOfFire", "Rate of fire"),
				FText::Format(LOCTEXT("RateOfFireInfoFormat", "{0} rpm"), FText::AsNumber(Desc->WeaponCharacteristics.GunCharacteristics.AmmoRate)),
				FFlareStyleSet::GetIcon("Rate"),
				ShowHelpers);
		}
		AddCharacteristicToBlock(Box,
			LOCTEXT("Magazine", "Magazine"),
			FText::AsNumber(Desc->WeaponCharacteristics.AmmoCapacity),
			FFlareStyleSet::GetIcon("Ammo"),
			ShowHelpers);
	}

	if (Desc->EngineCharacteristics.IsEngine)
	{
		if (Desc->EngineCharacteristics.AngularAccelerationRate > 0)
		{
			AddCharacteristicToBlock(Box,
				LOCTEXT("TurnRating", "Turn rating"),
				FText::AsNumber(Desc->EngineCharacteristics.AngularAccelerationRate),
				FFlareStyleSet::GetIcon("RCS"),
				ShowHelpers);
		}
		AddCharacteristicToBlock(Box,
			LOCTEXT("Thrust", "Thrust"),
			FText::Format(LOCTEXT("ThrustInfoFormat", "{0} kN"), FText::AsNumber(Desc->EngineCharacteristics.EnginePower)),
			FFlareStyleSet::GetIcon("Propulsion"),
			ShowHelpers);
	}
}

void SFlarePartInfo::AddCharacteristicToBlock(TSharedPtr<SHorizontalBox>& Box, FText Label, FText Value, const FSlateBrush* Icon, bool ShowHelpers)
{
	TSharedPtr<SVerticalBox> TempBox;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

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
					.TextStyle(&Theme.NameFont)
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
				.TextStyle(&Theme.SmallFont)
			];
	}
}

#undef LOCTEXT_NAMESPACE
