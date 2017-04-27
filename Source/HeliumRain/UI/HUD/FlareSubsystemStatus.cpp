
#include "FlareSubsystemStatus.h"
#include "../../Flare.h"
#include "../Components/FlareRoundButton.h"
#include "../../Spacecrafts/FlareWeapon.h"
#include "../../Spacecrafts/FlareSpacecraft.h"
#include "../../Player/FlarePlayerController.h"
#include "../../Player/FlareMenuManager.h"

#define LOCTEXT_NAMESPACE "FlareSubsystemStatus"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSubsystemStatus::Construct(const FArguments& InArgs)
{
	// Args
	MenuManager = InArgs._MenuManager;
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
	
	// Structure
	ChildSlot
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Center)
	[
		SNew(SFlareRoundButton)
		.Clickable(false)
		.Icon(Icon)
		.Text(this, &SFlareSubsystemStatus::GetText)
		.HelpText(this, &SFlareSubsystemStatus::GetInfoText)
		.HighlightColor(this, &SFlareSubsystemStatus::GetHealthColor)
		.TextColor(this, &SFlareSubsystemStatus::GetFlashColor)
	];
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareSubsystemStatus::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	UFlareSimulatedSpacecraft* PlayerShip = MenuManager->GetPC()->GetPlayerShip();

	if (PlayerShip)
	{
		// Update health
		ComponentHealth = PlayerShip->GetDamageSystem()->GetSubsystemHealth(SubsystemType);

		// Update flash
		TimeSinceFlash += InDeltaTime;
		if (ComponentHealth < 0.98 * Health)
		{
			TimeSinceFlash = 0;
			Health = ComponentHealth;
		}
		else if (ComponentHealth > Health)
		{
			Health = ComponentHealth;
		}
	}
	else
	{
		Health = 1.0f;
	}
}

FText SFlareSubsystemStatus::GetText() const
{
	UFlareSimulatedSpacecraft* PlayerShip = MenuManager->GetPC()->GetPlayerShip();
	FCHECK(PlayerShip);

	UFlareSimulatedSpacecraftDamageSystem* DamageSystem = PlayerShip->GetDamageSystem();
	FText SystemText = UFlareSimulatedSpacecraftDamageSystem::GetSubsystemName(SubsystemType);

	switch (SubsystemType)
	{
		case EFlareSubsystem::SYS_Propulsion:
			SystemText = DamageSystem->IsStranded() ? LOCTEXT("CockpitStranded", "STRANDED") : SystemText;
			break;

		case EFlareSubsystem::SYS_RCS:
			SystemText = DamageSystem->IsUncontrollable() ? LOCTEXT("CockpitUncontrollable", "UNCONTROLLABLE") : SystemText;
			break;

		case EFlareSubsystem::SYS_Weapon:
			SystemText = DamageSystem->IsDisarmed() ? LOCTEXT("CockpitDisarmed", "DISARMED") : SystemText;
			break;

		case EFlareSubsystem::SYS_Temperature:
			SystemText = (DamageSystem->GetTemperature() > DamageSystem->GetOverheatTemperature()) ? LOCTEXT("CockpitOverheating", "OVERHEATING") : SystemText;
			break;

		case EFlareSubsystem::SYS_Power:
			SystemText = DamageSystem->HasPowerOutage() ? LOCTEXT("CockpitOutage", "POWER OUTAGE") : SystemText;
			break;

		case EFlareSubsystem::SYS_LifeSupport:
			break;
	}

	// Format result
	return FText::Format(LOCTEXT("SubsystemInfoFormat", "{0}\n{1}%"),
		SystemText,
		FText::AsNumber(FMath::RoundToInt(100 * ComponentHealth)));
}

FText SFlareSubsystemStatus::GetInfoText() const
{
	FText Text;

	switch (SubsystemType)
	{
		case EFlareSubsystem::SYS_Temperature:
			Text = LOCTEXT("SYS_TemperatureInfo", "The cooling subsystem ensures that your ship doesn't overheat and burn.");
			break;
		case EFlareSubsystem::SYS_Propulsion:    
			Text = LOCTEXT("SYS_PropulsionInfo", "Orbital engines are required for orbital travel and can be used in combat to boost the spacecraft. They produce a lot of heat.");
			break;
		case EFlareSubsystem::SYS_RCS:           
			Text = LOCTEXT("SYS_RCSInfo", "The Reaction Control System is responsible for moving and turning the ship around in space. They produce little heat.");
			break;
		case EFlareSubsystem::SYS_LifeSupport:   
			Text = LOCTEXT("SYS_LifeSupportInfo", "The life support system needs to stay intact and powered in order for the crew to survive.");
			break;
		case EFlareSubsystem::SYS_Power:        
			Text = LOCTEXT("SYS_PowerInfo", "The power subsystem feeds energy to every system on your ship, including propulsion, weapons or life support.");
			break;
		case EFlareSubsystem::SYS_Weapon:        
			Text = LOCTEXT("SYS_WeaponInfo", "Weapons can be used to damage other spacecrafts. They produce a lot of heat.");         
			break;
	}

	return Text;
}

FSlateColor SFlareSubsystemStatus::GetHealthColor() const
{
	return FFlareStyleSet::GetHealthColor(ComponentHealth, false);
}

FSlateColor SFlareSubsystemStatus::GetFlashColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.NeutralColor;
	FLinearColor DamageColor = Theme.EnemyColor;

	FLinearColor Color = FMath::Lerp(DamageColor, NormalColor, FMath::Clamp(TimeSinceFlash / HealthDropFlashTime, 0.0f, 1.0f));
	Color.A *= Theme.DefaultAlpha;
	return Color;
}


#undef LOCTEXT_NAMESPACE
