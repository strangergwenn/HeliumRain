
#include "../../Flare.h"
#include "FlareSubsystemStatus.h"
#include "../Components/FlareLargeButton.h"
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

	// Icon
	const FSlateBrush* Icon = NULL;
	switch (SubsystemType)
	{
		case EFlareSubsystem::SYS_Temperature:   Icon = FFlareStyleSet::GetIcon("HUD_Temperature");  break;
		case EFlareSubsystem::SYS_Propulsion:    Icon = FFlareStyleSet::GetIcon("HUD_Propulsion");   break;
		case EFlareSubsystem::SYS_RCS:           Icon = FFlareStyleSet::GetIcon("HUD_RCS");          break;
		case EFlareSubsystem::SYS_LifeSupport:   Icon = FFlareStyleSet::GetIcon("HUD_LifeSupport");  break;
		case EFlareSubsystem::SYS_Power:         Icon = FFlareStyleSet::GetIcon("HUD_Power");        break;
		case EFlareSubsystem::SYS_Weapon:        Icon = FFlareStyleSet::GetIcon("HUD_Shell");        break;
	}

	// Text
	FText Text;
	switch (SubsystemType)
	{
		case EFlareSubsystem::SYS_Temperature:   Text = LOCTEXT("SYS_Temperature", "COOLING");      break;
		case EFlareSubsystem::SYS_Propulsion:    Text = LOCTEXT("SYS_Propulsion", "ENGINES");       break;
		case EFlareSubsystem::SYS_RCS:           Text = LOCTEXT("SYS_RCS", "RCS");                  break;
		case EFlareSubsystem::SYS_LifeSupport:   Text = LOCTEXT("SYS_LifeSupport", "CREW");         break;
		case EFlareSubsystem::SYS_Power:         Text = LOCTEXT("SYS_Power", "POWER");              break;
		case EFlareSubsystem::SYS_Weapon:        Text = LOCTEXT("SYS_Weapon", "WEAPONS");           break;
	}

	// Structure
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Center)
	[
		SNew(SFlareLargeButton)
		.Clickable(false)
		.Icon(Icon)
		.Text(Text)
		.IconColor(this, &SFlareSubsystemStatus::GetIconColor)
		.HighlightColor(this, &SFlareSubsystemStatus::GetHighlightColor)
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


#undef LOCTEXT_NAMESPACE
