
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
	DisplayType = InArgs._Type;
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
				.BorderImage(&Theme.BackgroundBrush)
				.Visibility(this, &SFlareSubsystemStatus::IsIconVisible)
				.Padding(FMargin(5, 0))
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
		[
			// Background
			SNew(SBorder)
			.BorderImage(&Theme.InvertedBrush)
			.BorderBackgroundColor(this, &::SFlareSubsystemStatus::GetFlashColor)
			.Padding(FMargin(5, 0))
			[
				SNew(STextBlock)
				.Text(this, &SFlareSubsystemStatus::GetTypeText)
				.TextStyle(&Theme.InvertedSmallFont)
				.Justification(ETextJustify::Center)
			]
		]

		// Status string
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			// Background
			SNew(SBorder)
			.BorderImage(&Theme.BackgroundBrush)
			.Padding(FMargin(5, 0))
			[
				SNew(STextBlock)
				.Text(this, &SFlareSubsystemStatus::GetStatusText)
				.TextStyle(&Theme.TextFont)
				.Justification(ETextJustify::Center)
			]
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
	
	if (TargetShip && DisplayType == EFlareInfoDisplay::ID_Subsystem)
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

EVisibility SFlareSubsystemStatus::IsIconVisible() const
{
	return (DisplayType == EFlareInfoDisplay::ID_Subsystem) ? EVisibility::Visible : EVisibility::Hidden;
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

FSlateColor SFlareSubsystemStatus::GetIconColor() const
{
	return FLinearColor(FColor::MakeRedToGreenColorFromScalar(ComponentHealth)).Desaturate(0.05);
}

FSlateColor SFlareSubsystemStatus::GetFlashColor() const
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
	if (Ship && DisplayType == EFlareInfoDisplay::ID_Subsystem)
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

	// Speed display
	else if (Ship && DisplayType == EFlareInfoDisplay::ID_Speed)
	{
		return FText::FromString(FString::FromInt(Ship->GetLinearVelocity().Size()) + " m/s");
	}

	// Sector display
	else if (DisplayType == EFlareInfoDisplay::ID_Sector)
	{
		return FText::FromString("Nema D43");
	}

	return FText::FromString("");
}

FText SFlareSubsystemStatus::GetTypeText() const
{
	// Subsystem display
	if (DisplayType == EFlareInfoDisplay::ID_Subsystem)
	{
		switch (SubsystemType)
		{
			case EFlareSubsystem::SYS_Temperature:   return LOCTEXT("SYS_Temperature", "COOLING");
			case EFlareSubsystem::SYS_Propulsion:    return LOCTEXT("SYS_Propulsion",  "MAIN ENGINES");
			case EFlareSubsystem::SYS_RCS:           return LOCTEXT("SYS_RCS",         "RCS");
			case EFlareSubsystem::SYS_LifeSupport:   return LOCTEXT("SYS_LifeSupport", "LIFE SUPPORT");
			case EFlareSubsystem::SYS_Power:         return LOCTEXT("SYS_Power",       "POWER");
			case EFlareSubsystem::SYS_Weapon:        return LOCTEXT("SYS_Weapon",      "WEAPON");
		}
	}

	// Speed display
	else if (DisplayType == EFlareInfoDisplay::ID_Speed)
	{
		return LOCTEXT("ShipSpeed", "SHIP SPEED");
	}

	// Sector display
	else if (DisplayType == EFlareInfoDisplay::ID_Sector)
	{
		return LOCTEXT("Sector", "SECTOR");
	}

	return FText::FromString("");
}


#undef LOCTEXT_NAMESPACE
