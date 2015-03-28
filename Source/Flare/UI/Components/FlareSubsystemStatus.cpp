
#include "../../Flare.h"
#include "FlareSubsystemStatus.h"
#include "../Components/FlareButton.h"
#include "../../Ships/FlareWeapon.h"
#include "../../Ships/FlareShip.h"

#define LOCTEXT_NAMESPACE "FlareSubsystemStatus"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSubsystemStatus::Construct(const FArguments& InArgs)
{
	// Data
	TargetShip = NULL;
	TargetComponent = NULL;
	SubsystemType = InArgs._Subsystem;

	// Effect data
	Health = 1.0f;
	HealthDropFlashTime = 2.0f;
	TimeSinceFlash = HealthDropFlashTime;

	// Style
	const FFlareButtonStyle* ButtonStyle = &FFlareStyleSet::Get().GetWidgetStyle<FFlareButtonStyle>("/Style/HUDIndicatorIcon");
	const FFlareContainerStyle* ContainerStyle = &FFlareStyleSet::Get().GetWidgetStyle<FFlareContainerStyle>("/Style/DefaultContainerStyle");
	const FFlareContainerStyle* InvertedContainerStyle = &FFlareStyleSet::Get().GetWidgetStyle<FFlareContainerStyle>("/Style/InvertedContainerStyle");

	// Structure
	ChildSlot
	.VAlign(VAlign_Top)
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
			.BorderBackgroundColor(this, &::SFlareSubsystemStatus::GetFlashColor)
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

void SFlareSubsystemStatus::SetTargetComponent(UFlareShipComponent* Target)
{
	TargetComponent = Target;
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareSubsystemStatus::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	
	// Update health
	TimeSinceFlash += InDeltaTime;
	float NewHealth = TargetShip->GetSubsystemHealth(SubsystemType);
	if (NewHealth < Health)
	{
		TimeSinceFlash = 0;
	}
	Health = NewHealth;
}

const FSlateBrush* SFlareSubsystemStatus::GetIcon() const
{
	switch (SubsystemType)
	{
		case EFlareSubsystem::SYS_Temperature:   return FFlareStyleSet::GetIcon("HUD_Temperature");
		case EFlareSubsystem::SYS_Propulsion:    return FFlareStyleSet::GetIcon("HUD_Propulsion");
		case EFlareSubsystem::SYS_RCS:           return FFlareStyleSet::GetIcon("HUD_RCS");
		case EFlareSubsystem::SYS_LifeSupport:   return FFlareStyleSet::GetIcon("HUD_LifeSupport");
		case EFlareSubsystem::SYS_Power:         return FFlareStyleSet::GetIcon("HUD_Power");
		case EFlareSubsystem::SYS_Weapon:        return FFlareStyleSet::GetIcon("HUD_Shell");
		default:                                 return FFlareStyleSet::GetIcon("HUD_LifeSupport");
	}
}

FSlateColor SFlareSubsystemStatus::GetIconColor() const
{
	return FLinearColor(FColor::MakeRedToGreenColorFromScalar(Health)).Desaturate(0.05);
}

FSlateColor SFlareSubsystemStatus::GetFlashColor() const
{
	FLinearColor FlashColor = FFlareStyleSet::GetHeatColor();
	float Ratio = FMath::Clamp(TimeSinceFlash / HealthDropFlashTime, 0.0f, 1.0f);
	return FMath::Lerp(FlashColor, FLinearColor::White, Ratio);
}

FText SFlareSubsystemStatus::GetStatusText() const
{
	// Data check
	AFlareShip* Ship = Cast<AFlareShip>(TargetShip);
	if (!TargetShip || !Ship)
	{
		return FText::FromString("?");
	}
	FString Text = (Health <= 0) ? LOCTEXT("Offline", "OFFLINE").ToString() : FString::FromInt(100 * Health) + " %";
	Text += "\n";

	// Advanced information
	switch (SubsystemType)
	{
		// Temperature display
		case EFlareSubsystem::SYS_Temperature:
			Text += FString::FromInt(TargetShip->GetTemperature()) + " K";
			break;

		// Ammo display
		case EFlareSubsystem::SYS_Weapon:
			if (TargetComponent)
			{
				UFlareWeapon* Weapon = Cast<UFlareWeapon>(TargetComponent);
				if (Weapon)
				{
					Text += FString::FromInt(Weapon->GetCurrentAmmo());
				}
			}
			break;

		// Power outages
		case EFlareSubsystem::SYS_Power:
			if (TargetShip->HasPowerOutage())
			{
				Text += LOCTEXT("PwBackIn", "Back in ").ToString() + FString::FromInt(TargetShip->GetPowerOutageDuration()) + " s";
			}
			break; 

		// Pilot mode
		case EFlareSubsystem::SYS_RCS:
			switch (Ship->GetCommandType())
			{
				case EFlareShipStatus::SS_Manual:    Text += LOCTEXT("CmdManual", "Manual").ToString();  break;
				case EFlareShipStatus::SS_Gliding:   Text += LOCTEXT("CmdGlide", "Gliding").ToString();  break;
				case EFlareShipStatus::SS_AutoPilot: Text += LOCTEXT("CmdAuto", "Autopilot").ToString(); break;
				case EFlareShipStatus::SS_Docked:    Text += LOCTEXT("CmdDocked", "Docked").ToString();  break;
			}
			break;

		// Boost mode
		case EFlareSubsystem::SYS_Propulsion:
			if (Ship->IsBoosting())
			{
				Text += LOCTEXT("Boosting", "Boosting").ToString();
			}
			break;

		// No particular information
		case EFlareSubsystem::SYS_LifeSupport:
		default:
			break;
	}

	return FText::FromString(Text);
}

FText SFlareSubsystemStatus::GetTypeText() const
{
	switch (SubsystemType)
	{
		case EFlareSubsystem::SYS_Temperature:   return LOCTEXT("SYS_Temperature", "COOLING");
		case EFlareSubsystem::SYS_Propulsion:    return LOCTEXT("SYS_Propulsion",  "MAIN ENGINES");
		case EFlareSubsystem::SYS_RCS:           return LOCTEXT("SYS_RCS",         "RCS");
		case EFlareSubsystem::SYS_LifeSupport:   return LOCTEXT("SYS_LifeSupport", "LIFE SUPPORT");
		case EFlareSubsystem::SYS_Power:         return LOCTEXT("SYS_Power",       "POWER");
		case EFlareSubsystem::SYS_Weapon:        return LOCTEXT("SYS_Weapon",      "WEAPON");
		default:                                 return LOCTEXT("SYS_Default",     "HULL");
	}
}


#undef LOCTEXT_NAMESPACE
