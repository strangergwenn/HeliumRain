
#include "../../Flare.h"
#include "FlareSubsystemStatus.h"
#include "../Components/FlareButton.h"

#define LOCTEXT_NAMESPACE "FlareSubsystemStatus"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSubsystemStatus::Construct(const FArguments& InArgs)
{
	// Data
	TargetShip = NULL;
	SubsystemType = InArgs._Subsystem;
	const FFlareButtonStyle* ButtonStyle = &FFlareStyleSet::Get().GetWidgetStyle<FFlareButtonStyle>("/Style/HUDIndicatorIcon");
	const FFlareContainerStyle* ContainerStyle = &FFlareStyleSet::Get().GetWidgetStyle<FFlareContainerStyle>("/Style/DefaultContainerStyle");
	const FFlareContainerStyle* InvertedContainerStyle = &FFlareStyleSet::Get().GetWidgetStyle<FFlareContainerStyle>("/Style/InvertedContainerStyle");

	// Structure
	ChildSlot
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Center)
	[
		SNew(SVerticalBox)

		// Icon
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0))
		[
			// Content box
			SNew(SBox)
			.WidthOverride(ButtonStyle->Width)
			.HeightOverride(ButtonStyle->Height)
			[
				// Background
				SNew(SBorder)
				.Padding(ContainerStyle->BorderPadding)
				.BorderImage(&ContainerStyle->BackgroundBrush)
				[
					SNew(SImage)
					.Image(this, &SFlareSubsystemStatus::GetIcon)
					.ColorAndOpacity(this, &SFlareSubsystemStatus::GetIconColor)
				]
			]
		]

		// Subsystem type
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0))
		[
			// Background
			SNew(SBorder)
			.Padding(FMargin(ButtonStyle->ContentPadding))
			.BorderImage(&InvertedContainerStyle->BackgroundBrush)
			[
				SNew(STextBlock)
				.Text(this, &SFlareSubsystemStatus::GetTypeText)
				.TextStyle(FFlareStyleSet::Get(), "Flare.VerySmallTextInverted")
				.Justification(ETextJustify::Center)
			]
		]

		// Status string
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0))
		[
			// Background
			SNew(SBorder)
			.Padding(FMargin(ButtonStyle->ContentPadding))
			.BorderImage(&ContainerStyle->BackgroundBrush)
			[
				SNew(STextBlock)
				.Text(this, &SFlareSubsystemStatus::GetStatusText)
				.TextStyle(FFlareStyleSet::Get(), "Flare.Text")
				.Justification(ETextJustify::Center)
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareSubsystemStatus::SetTargetShip(IFlareShipInterface* Target)
{
	TargetShip = Target;
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

const FSlateBrush* SFlareSubsystemStatus::GetIcon() const
{
	switch (SubsystemType)
	{
		case EFlareSubsystem::SYS_Temperature:   return FFlareStyleSet::GetIcon("HUD_Temperature");
		case EFlareSubsystem::SYS_Propulsion:    return FFlareStyleSet::GetIcon("HUD_Propulsion");
		case EFlareSubsystem::SYS_RCS:           return FFlareStyleSet::GetIcon("HUD_RCS");
		case EFlareSubsystem::SYS_LifeSupport:   return FFlareStyleSet::GetIcon("HUD_LifeSupport");
		case EFlareSubsystem::SYS_Power:         return FFlareStyleSet::GetIcon("HUD_Power");
		case EFlareSubsystem::SYS_Gun:           return FFlareStyleSet::GetIcon("HUD_Gun");
		case EFlareSubsystem::SYS_Turret:        return FFlareStyleSet::GetIcon("HUD_Turret");
		default:                                 return FFlareStyleSet::GetIcon("HUD_LifeSupport");
	}
}

FSlateColor SFlareSubsystemStatus::GetIconColor() const
{
	return FLinearColor(FColor::MakeRedToGreenColorFromScalar(1.0)).Desaturate(0.05);
}

FText SFlareSubsystemStatus::GetStatusText() const
{
	FString Text;

	switch (SubsystemType)
	{
		case EFlareSubsystem::SYS_Temperature:
			Text = "150 K";
			break;

		case EFlareSubsystem::SYS_Propulsion:
			Text = "15 m/s";
			break;

		case EFlareSubsystem::SYS_RCS:
			Text = "5 deg/s";
			break;

		case EFlareSubsystem::SYS_Gun:
		case EFlareSubsystem::SYS_Turret:
			Text = "150 / 150";
			break;

		case EFlareSubsystem::SYS_LifeSupport:
		case EFlareSubsystem::SYS_Power:
		default:
			Text = "100 %";
			break;
	}

	return FText::FromString(Text);
}

FText SFlareSubsystemStatus::GetTypeText() const
{
	switch (SubsystemType)
	{
		case EFlareSubsystem::SYS_Temperature:   return LOCTEXT("SYS_Temperature", "COOLING");
		case EFlareSubsystem::SYS_Propulsion:    return LOCTEXT("SYS_Propulsion",  "PROPULSION");
		case EFlareSubsystem::SYS_RCS:           return LOCTEXT("SYS_RCS",         "RCS");
		case EFlareSubsystem::SYS_LifeSupport:   return LOCTEXT("SYS_LifeSupport", "LIFE SUPPORT");
		case EFlareSubsystem::SYS_Power:         return LOCTEXT("SYS_Power",       "POWER");
		case EFlareSubsystem::SYS_Gun:           return LOCTEXT("SYS_Weapon",      "GUN");
		case EFlareSubsystem::SYS_Turret:        return LOCTEXT("SYS_Weapon",      "TURRET");
		default:                                 return LOCTEXT("SYS_Default",     "HULL");
	}
}


#undef LOCTEXT_NAMESPACE
