
#include "../../Flare.h"
#include "FlareSubsystemStatus.h"
#include "../Components/FlareButton.h"
#include "../../Spacecrafts/FlareWeapon.h"
#include "../../Spacecrafts/FlareSpacecraft.h"

#define LOCTEXT_NAMESPACE "FlareSubsystemStatus"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSubsystemStatus::Construct(const FArguments& InArgs)
{
	// Args
	TargetShip = NULL;
	TargetComponent = NULL;
	SubsystemType = InArgs._Subsystem;

	// Settings
	Health = 1.0f;
	ComponentHealth = 1.0f;
	HealthDropFlashTime = 2.0f;
	TimeSinceFlash = HealthDropFlashTime;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	// Structure
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Center)
	[
		SNew(SVerticalBox)

		// Icon
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			// Content box
			SNew(SBox)
			.WidthOverride(Theme.LargeButtonWidth)
			.HeightOverride(Theme.LargeButtonHeight)
			[
				// Background
				SNew(SBorder)
				.Padding(Theme.LargeButtonPadding)
				.BorderImage(&Theme.LargeButtonBackground)
				.BorderBackgroundColor(this, &SFlareSubsystemStatus::GetHighlightColor)
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
		.Padding(Theme.ContentPadding)
		[
			SNew(STextBlock)
			.Text(this, &SFlareSubsystemStatus::GetTypeText)
			.TextStyle(&Theme.SmallFont)
			.Justification(ETextJustify::Center)
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareSubsystemStatus::SetTargetShip(IFlareSpacecraftInterface* Target)
{
	TargetShip = Target;
}

void SFlareSubsystemStatus::SetTargetComponent(UFlareSpacecraftComponent* Target)
{
	TargetComponent = Target;
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareSubsystemStatus::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	
	if (TargetShip)
	{
		// Update health
		float NewHealth = TargetShip->GetDamageSystem()->GetSubsystemHealth(SubsystemType, true);
		ComponentHealth = TargetShip->GetDamageSystem()->GetSubsystemHealth(SubsystemType);

		// Update flash
		TimeSinceFlash += InDeltaTime;
		if (NewHealth < Health)
		{
			TimeSinceFlash = 0;
		}
		Health = NewHealth;
	}
	else
	{
		Health = 1;
	}
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
	}

	return NULL;
}

FSlateColor SFlareSubsystemStatus::GetHighlightColor() const
{
	FLinearColor NormalColor = FFlareStyleSet::GetDefaultTheme().NeutralColor;
	FLinearColor DamageColor = FFlareStyleSet::GetDefaultTheme().EnemyColor;
	return FMath::Lerp(DamageColor, NormalColor, ComponentHealth);
}

FSlateColor SFlareSubsystemStatus::GetIconColor() const
{
	FLinearColor FlashColor = FFlareStyleSet::GetDefaultTheme().EnemyColor;
	FLinearColor NeutralColor = FFlareStyleSet::GetDefaultTheme().NeutralColor;

	float Ratio = FMath::Clamp(TimeSinceFlash / HealthDropFlashTime, 0.0f, 1.0f);
	return FMath::Lerp(FlashColor, NeutralColor, Ratio);
}

FText SFlareSubsystemStatus::GetStatusText() const
{
	// Data check
	AFlareSpacecraft* Ship = Cast<AFlareSpacecraft>(TargetShip);

	// Subsystem display
	if (Ship)
	{
		FString Text;

		switch (SubsystemType)
		{
			// Temperature display
			case EFlareSubsystem::SYS_Temperature:
				Text = FString::FromInt(Ship->GetDamageSystem()->GetTemperature()) + " K";
				break;

			// Ammo display
			case EFlareSubsystem::SYS_Weapon:
				if (TargetComponent)
				{
					UFlareWeapon* Weapon = Cast<UFlareWeapon>(TargetComponent);
					if (Weapon)
					{
						Text = FString::FromInt(Weapon->GetCurrentAmmo());
					}
				}
				break;

			// Power outages
			case EFlareSubsystem::SYS_Power:
				if (Ship->GetDamageSystem()->HasPowerOutage())
				{
					Text = LOCTEXT("PwBackIn", "Back in ").ToString() + FString::FromInt(Ship->GetDamageSystem()->GetPowerOutageDuration() + 1) + " s";
				}
				break; 

			// Pilot mode
			case EFlareSubsystem::SYS_RCS:
				switch (Ship->GetNavigationSystem()->GetStatus())
				{
					case EFlareShipStatus::SS_Manual:    Text += LOCTEXT("CmdManual", "Manual").ToString();  break;
					case EFlareShipStatus::SS_AutoPilot: Text += LOCTEXT("CmdAuto", "Autopilot").ToString(); break;
					case EFlareShipStatus::SS_Docked:    Text += LOCTEXT("CmdDocked", "Docked").ToString();  break;
				}
				break;

			// Boost mode
			case EFlareSubsystem::SYS_Propulsion:
				if (Ship->GetNavigationSystem()->IsBoosting())
				{
					Text = LOCTEXT("Boosting", "Boosting").ToString();
				}
				break;

			// No particular information
			case EFlareSubsystem::SYS_LifeSupport:
			default:
				break;
		}

		return FText::FromString(Text);
	}

	return FText::FromString("");
}

FText SFlareSubsystemStatus::GetTypeText() const
{
	// Subsystem display
	switch (SubsystemType)
	{
		case EFlareSubsystem::SYS_Temperature:   return LOCTEXT("SYS_Temperature", "COOLING");
		case EFlareSubsystem::SYS_Propulsion:    return LOCTEXT("SYS_Propulsion",  "ENGINES");
		case EFlareSubsystem::SYS_RCS:           return LOCTEXT("SYS_RCS",         "RCS");
		case EFlareSubsystem::SYS_LifeSupport:   return LOCTEXT("SYS_LifeSupport", "CREW");
		case EFlareSubsystem::SYS_Power:         return LOCTEXT("SYS_Power",       "POWER");
		case EFlareSubsystem::SYS_Weapon:        return LOCTEXT("SYS_Weapon",      "WEAPONS");
	}

	return FText::FromString("");
}


#undef LOCTEXT_NAMESPACE
