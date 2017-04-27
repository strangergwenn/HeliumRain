
#include "FlareHUDMenu.h"
#include "../../Flare.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareMenuManagerMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareHUDMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	Overheating = false;
	PowerOutage = false;
	PresentationFlashTime = 0.2f;
	TimeSinceOverheatChanged = PresentationFlashTime;
	TimeSinceOutageChanged = PresentationFlashTime;
	AFlarePlayerController* PC = MenuManager->GetPC();

	// Style
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.NeutralColor;
	FLinearColor EnemyColor = Theme.EnemyColor;
	NormalColor.A = Theme.DefaultAlpha;
	EnemyColor.A = Theme.DefaultAlpha;

	// Structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0))
	[
		SNew(SVerticalBox)

		// Info text
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		[
			SAssignNew(InfoText, STextBlock)
			.TextStyle(&Theme.NameFont)
			.Justification(ETextJustify::Center)
			.Text(this, &SFlareHUDMenu::GetInfoText)
			.ColorAndOpacity(NormalColor)
			.Visibility(this, &SFlareHUDMenu::GetTopPanelVisibility)
		]

		// Info text 2
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		[
			SAssignNew(LowerInfoText, STextBlock)
			.TextStyle(&Theme.NameFont)
			.Text(this, &SFlareHUDMenu::GetWarningText)
			.ColorAndOpacity(EnemyColor)
			.Visibility(this, &SFlareHUDMenu::GetTopPanelVisibility)
		]
	
		// Overheating box
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(Theme.ContentPadding)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.SubTitleFont)
			.Text(LOCTEXT("Overheating", "OVERHEATING !"))
			.ColorAndOpacity(this, &SFlareHUDMenu::GetOverheatColor, true)
		]

		// Outage box
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(Theme.ContentPadding)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.SubTitleFont)
				.Text(LOCTEXT("PowerOutage", "POWER OUTAGE !"))
				.ColorAndOpacity(this, &SFlareHUDMenu::GetOutageColor, true)
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(this, &SFlareHUDMenu::GetOutageText)
				.TextStyle(&Theme.SubTitleFont)
				.ColorAndOpacity(this, &SFlareHUDMenu::GetOutageColor, true)
			]
		]

		// Weapon panel
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(0, 0, 0, 0))
		[
			SAssignNew(WeaponContainer, SVerticalBox)
		]

		// Status panel
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		[
			SNew(SHorizontalBox)
			
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(TemperatureStatus, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_Temperature)
				.MenuManager(MenuManager)
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(PowerStatus, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_Power)
				.MenuManager(MenuManager)
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(PropulsionStatus, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_Propulsion)
				.MenuManager(MenuManager)
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(RCSStatus, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_RCS)
				.MenuManager(MenuManager)
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(LifeSupportStatus, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_LifeSupport)
				.MenuManager(MenuManager)
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(WeaponStatus, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_Weapon)
				.MenuManager(MenuManager)
			]
		]

		// Overheating progress bar
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		[
			SNew(SHorizontalBox)
				
			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(Theme.SmallContentPadding)
			[
				SNew(SImage)
				.Image(FFlareStyleSet::GetIcon("Temperature"))
				.ColorAndOpacity(this, &SFlareHUDMenu::GetTemperatureColorNoAlpha)
			]

			// Bar
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.MinDesiredWidth(500)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				[
					SNew(SProgressBar)
					.Style(&Theme.ProgressBarStyle)
					.Percent(this, &SFlareHUDMenu::GetTemperatureProgress)
					.FillColorAndOpacity(this, &SFlareHUDMenu::GetTemperatureColorNoAlpha)
				]
			]

			// Text
			+ SHorizontalBox::Slot()
			.Padding(Theme.SmallContentPadding)
			[
				SNew(SBox)
				.MinDesiredWidth(100)
				.Padding(Theme.SmallContentPadding)
				.VAlign(VAlign_Top)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.NameFont)
					.Text(this, &SFlareHUDMenu::GetTemperatureText)
					.ColorAndOpacity(this, &SFlareHUDMenu::GetTemperatureColor)
				]
			]
		]
	];

	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareHUDMenu::OnPlayerShipChanged()
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Is this a civilian ship ?
	UFlareSimulatedSpacecraft* PlayerShip = MenuManager->GetPC()->GetPlayerShip();
	WeaponStatus->SetVisibility(PlayerShip->IsMilitary() ? EVisibility::Visible : EVisibility::Hidden);
	WeaponContainer->ClearChildren();

	// Update weapon list
	if (PlayerShip && PlayerShip->IsMilitary())
	{
		TArray<FFlareWeaponGroup*>& WeaponGroupList = PlayerShip->GetActive()->GetWeaponsSystem()->GetWeaponGroupList();

		// Add weapon indicators
		for (int32 i = WeaponGroupList.Num() - 1; i >= 0; i--)
		{
			WeaponContainer->AddSlot()
			.AutoHeight()
			[
				SNew(SFlareWeaponStatus)
				.MenuManager(MenuManager)
				.TargetWeaponGroupIndex(i)
			];
		}

		// No weapon
		WeaponContainer->AddSlot()
		.AutoHeight()
		[
			SNew(SFlareWeaponStatus)
			.MenuManager(MenuManager)
			.TargetWeaponGroupIndex(-1)
		];
	}
}


/*----------------------------------------------------
	Events
----------------------------------------------------*/

void SFlareHUDMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	UFlareSimulatedSpacecraft* PlayerShip = MenuManager->GetPC()->GetPlayerShip();

	if (PlayerShip)
	{
		// Is alive ?
		SetVisibility(PlayerShip->GetDamageSystem()->IsAlive() ? EVisibility::Visible : EVisibility::Collapsed);

		// Overheating status
		TimeSinceOverheatChanged += InDeltaTime;
		Temperature = PlayerShip->GetDamageSystem()->GetTemperature();
		OverheatTemperature = PlayerShip->GetDamageSystem()->GetOverheatTemperature();

		// Alert the player if the ship is near the overheat temperature
		bool NewOverheating = (PlayerShip->GetDamageSystem()->GetTemperature() > PlayerShip->GetDamageSystem()->GetOverheatTemperature() * 0.95);
		if (NewOverheating != Overheating)
		{
			TimeSinceOverheatChanged = 0;
		}
		Overheating = NewOverheating;

		// Outage status
		TimeSinceOutageChanged += InDeltaTime;
		bool NewPowerOutage = PlayerShip->GetDamageSystem()->HasPowerOutage();
		if (NewPowerOutage != PowerOutage)
		{
			TimeSinceOutageChanged = 0;
		}
		PowerOutage = NewPowerOutage;
	}
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

EVisibility SFlareHUDMenu::GetTopPanelVisibility() const
{
	return MenuManager->IsOverlayOpen() ? EVisibility::Hidden : EVisibility::Visible;
}

FText SFlareHUDMenu::GetInfoText() const
{
	UFlareSimulatedSpacecraft* PlayerShip = MenuManager->GetPC()->GetPlayerShip();

	if (PlayerShip && PlayerShip->IsActive() && !MenuManager->GetPC()->UseCockpit && MenuManager->GetPC()->GetGame()->GetActiveSector())
	{
		AFlareSpacecraft* ActivePlayerShip = PlayerShip->GetActive();

		// Get mode info
		FText ModeText;
		FText AutopilotText;
		if (ActivePlayerShip->GetNavigationSystem()->IsDocked())
		{
			ModeText = LOCTEXT("Docked", "Docked");
		}
		else
		{
			ModeText = ActivePlayerShip->GetWeaponsSystem()->GetWeaponModeInfo();
			if (ActivePlayerShip->GetNavigationSystem()->IsAutoPilot())
			{
				AutopilotText = LOCTEXT("AUTOPILOT", " (Autopilot)");
			}
		}
		
		// Sector info
		UFlareSimulatedSector* CurrentSector = PlayerShip->GetGame()->GetActiveSector()->GetSimulatedSector();
		FText SectorText = FText::Format(LOCTEXT("CurrentSectorFormat", "{0} ({1})"),
			CurrentSector->GetSectorName(),
			CurrentSector->GetSectorFriendlynessText(PlayerShip->GetCompany()));

		// Full flight info
		FText FlightInfo = FText::Format(LOCTEXT("ShipInfoTextFormat", "{0}m/s - {1} {2} in {3}"),
			FText::AsNumber(FMath::RoundToInt(ActivePlayerShip->GetLinearVelocity().Size())),
			ModeText,
			AutopilotText,
			SectorText);

		// Battle status
		FText BattleText = CurrentSector->GetSectorBattleStateText(MenuManager->GetPC()->GetCompany());

		// Target info
		FText TargetText;
		AFlareSpacecraft* TargetShip = PlayerShip->GetActive()->GetCurrentTarget();
		if (TargetShip && TargetShip->IsValidLowLevel())
		{
			TargetText = FText::Format(LOCTEXT("CurrentTargetFormat", "Targeting {0}"),
				UFlareGameTools::DisplaySpacecraftName(TargetShip->GetParent()));
		}
		else
		{
			TargetText = LOCTEXT("CurrentNoTarget", "No target");
		}

		// Build result
		FString Result = FlightInfo.ToString() + " - " + TargetText.ToString() + "\n" + CurrentSector->GetSectorBalanceText(true).ToString();
		if (BattleText.ToString().Len())
		{
			Result += " - " + BattleText.ToString();
		}

		// Add performance
		FText PerformanceText = MenuManager->GetPC()->GetNavHUD()->GetPerformanceText();
		if (PerformanceText.ToString().Len())
		{
			Result += "\n" + PerformanceText.ToString();
		}

		return FText::FromString(Result);
	}

	return FText();
}

FText SFlareHUDMenu::GetWarningText() const
{
	FString Info;

	UFlareSimulatedSpacecraft* PlayerShip = MenuManager->GetPC()->GetPlayerShip();
	UFlareSimulatedSector* CurrentSector = PlayerShip->GetGame()->GetActiveSector()->GetSimulatedSector();

	if (CurrentSector)
	{
		// Get player threats
		bool Targeted, FiredUpon, CollidingSoon, ExitingSoon, LowHealth;
		UFlareSimulatedSpacecraft* Threat;
		MenuManager->GetPC()->GetPlayerShipThreatStatus(Targeted, FiredUpon, CollidingSoon, ExitingSoon, LowHealth, Threat);

		// Fired on ?
		if (FiredUpon)
		{
			if (Threat)
			{
				FText WarningText = FText::Format(LOCTEXT("ThreatFiredUponFormat", "UNDER FIRE FROM {0} ({1})"),
					UFlareGameTools::DisplaySpacecraftName(Threat),
					FText::FromString(Threat->GetCompany()->GetShortName().ToString()));
				Info = WarningText.ToString();
			}
			else
			{
				FText WarningText = FText(LOCTEXT("ThreatFiredUponMissile", "INCOMING MISSILE"));
				Info = WarningText.ToString();
			}
		}

		// Collision ?
		else if (CollidingSoon)
		{
			FText WarningText = LOCTEXT("ThreatCollisionFormat", "IMMINENT COLLISION");
			Info = WarningText.ToString();
		}

		// Leaving sector ?
		else if (ExitingSoon)
		{
			FText WarningText = LOCTEXT("ThreatLeavingFormat", "LEAVING SECTOR");
			Info = WarningText.ToString();
		}

		// Targeted ?
		else if (Targeted)
		{
			FText WarningText = FText::Format(LOCTEXT("ThreatTargetFormat", "TARGETED BY {0} ({1})"),
				UFlareGameTools::DisplaySpacecraftName(Threat),
				FText::FromString(Threat->GetCompany()->GetShortName().ToString()));
			Info = WarningText.ToString();
		}

		// Low health
		else if (LowHealth)
		{
			FText WarningText = LOCTEXT("ThreatHealthFormat", "LIFE SUPPORT COMPROMISED");
			Info = WarningText.ToString();
		}
	}

	return FText::FromString(Info);
}

TOptional<float> SFlareHUDMenu::GetTemperatureProgress() const
{
	float WidgetMin = 500.0f;
	float WidgetRange = OverheatTemperature - WidgetMin;
	return ((Temperature - WidgetMin) / WidgetRange);
}

FSlateColor SFlareHUDMenu::GetTemperatureColor() const
{
	float Distance = Temperature - 0.9f * OverheatTemperature;
	float Ratio = FMath::Clamp(FMath::Abs(Distance) / 10.0f, 0.0f, 1.0f);

	if (Distance < 0)
	{
		Ratio = 0.0f;
	}

	return FFlareStyleSet::GetHealthColor(1 - Ratio, true);
}

FSlateColor SFlareHUDMenu::GetTemperatureColorNoAlpha() const
{
	float Distance = Temperature - 0.9f * OverheatTemperature;
	float Ratio = FMath::Clamp(FMath::Abs(Distance) / 10.0f, 0.0f, 1.0f);

	if (Distance < 0)
	{
		Ratio = 0.0f;
	}

	return FFlareStyleSet::GetHealthColor(1 - Ratio, false);
}

FText SFlareHUDMenu::GetTemperatureText() const
{
	return FText::Format(LOCTEXT("TemperatureFormat", "{0} K"), FText::AsNumber((int32)Temperature));
}

FSlateColor SFlareHUDMenu::GetOverheatColor(bool Text) const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	FLinearColor Color = Theme.EnemyColor;
	float Ratio = FMath::Clamp(TimeSinceOverheatChanged / PresentationFlashTime, 0.0f, 1.0f);
	Color.A *= (Overheating ? Ratio : (1 - Ratio));

	if (Text)
	{
		Color.A *= Theme.DefaultAlpha;
	}

	return Color;
}

FSlateColor SFlareHUDMenu::GetOutageColor(bool Text) const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	FLinearColor Color = Theme.NeutralColor;
	float Ratio = FMath::Clamp(TimeSinceOutageChanged / PresentationFlashTime, 0.0f, 1.0f);
	Color.A *= (PowerOutage ? Ratio : (1 - Ratio));

	if (Text)
	{
		Color.A *= Theme.DefaultAlpha;
	}

	return Color;
}

FText SFlareHUDMenu::GetOutageText() const
{
	FText Result;

	UFlareSimulatedSpacecraft* PlayerShip = MenuManager->GetPC()->GetPlayerShip();

	if (PlayerShip)
	{
		return FText::Format(LOCTEXT("PwBackInFormat", "Power back in {0}..."),
			FText::AsNumber((int32)(PlayerShip->GetDamageSystem()->GetPowerOutageDuration()) + 1));
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE
