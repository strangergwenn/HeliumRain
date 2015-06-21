
#include "../../Flare.h"
#include "FlareSubsystemStatus.h"
#include "../Components/FlareRoundButton.h"
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
	ComponentHealth = 0.0f;
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
	
	// Structure
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Center)
	[
		SNew(SFlareRoundButton)
		.Clickable(false)
		.Icon(Icon)
		.Text(this, &SFlareSubsystemStatus::GetText)
		.HighlightColor(this, &SFlareSubsystemStatus::GetHealthColor)
		.TextColor(this, &SFlareSubsystemStatus::GetFlashColor)
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
		float NewHealth = TargetShip->GetDamageSystem()->GetSubsystemHealth(SubsystemType, true, false);
		ComponentHealth = TargetShip->GetDamageSystem()->GetSubsystemHealth(SubsystemType);

		// Update flash
		TimeSinceFlash += InDeltaTime;
		if (NewHealth < 0.98 * Health)
		{
			TimeSinceFlash = 0;
			Health = NewHealth;
		}
	}
	else
	{
		Health = 1;
	}
}

FText SFlareSubsystemStatus::GetText() const
{
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

	return FText::FromString(Text.ToString() + "\n" + FString::FromInt(100 * ComponentHealth) + "%");
}

FSlateColor SFlareSubsystemStatus::GetHealthColor() const
{
	return FFlareStyleSet::GetHealthColor(ComponentHealth, false);
}

FSlateColor SFlareSubsystemStatus::GetFlashColor() const
{
	return FFlareStyleSet::GetHealthColor(FMath::Clamp(TimeSinceFlash / HealthDropFlashTime, 0.0f, 1.0f), true);
}


#undef LOCTEXT_NAMESPACE
