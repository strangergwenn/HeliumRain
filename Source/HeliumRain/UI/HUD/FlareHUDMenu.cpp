
#include "../../Flare.h"
#include "FlareHUDMenu.h"
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
	TargetShip = NULL;
	Overheating = false;
	PowerOutage = false;
	PresentationFlashTime = 0.2f;
	TimeSinceOverheatChanged = PresentationFlashTime;
	TimeSinceOutageChanged = PresentationFlashTime;
	AFlarePlayerController* PC = MenuManager->GetPC();

	// Style
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.NeutralColor;
	NormalColor.A = Theme.DefaultAlpha;

	// Structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0))
	[
		SNew(SVerticalBox)

		// Text notification box
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		[
			SNew(SVerticalBox)

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
					.Visibility(this, &SFlareHUDMenu::GetTopPanelVisibility)
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
						.Visibility(this, &SFlareHUDMenu::GetTopPanelVisibility)
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
						.Visibility(this, &SFlareHUDMenu::GetTopPanelVisibility)
					]
				]
			]
		]

		// Info text
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		[
			SAssignNew(InfoText, STextBlock)
			.TextStyle(&Theme.NameFont)
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
			.Text(this, &SFlareHUDMenu::GetLowerInfoText)
			.ColorAndOpacity(NormalColor)
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
				SAssignNew(TemperatureStatus, SFlareSubsystemStatus).Subsystem(EFlareSubsystem::SYS_Temperature)
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(PowerStatus, SFlareSubsystemStatus).Subsystem(EFlareSubsystem::SYS_Power)
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(PropulsionStatus, SFlareSubsystemStatus).Subsystem(EFlareSubsystem::SYS_Propulsion)
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(RCSStatus, SFlareSubsystemStatus).Subsystem(EFlareSubsystem::SYS_RCS)
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(LifeSupportStatus, SFlareSubsystemStatus).Subsystem(EFlareSubsystem::SYS_LifeSupport)
			]

			+ SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(WeaponStatus, SFlareSubsystemStatus).Subsystem(EFlareSubsystem::SYS_Weapon)
			]
		]
	];

	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareHUDMenu::SetTargetShip(UFlareSimulatedSpacecraft* Target)
{
	// Set targets
	TargetShip = Target;
	TemperatureStatus->SetTargetShip(Target);
	PowerStatus->SetTargetShip(Target);
	PropulsionStatus->SetTargetShip(Target);
	RCSStatus->SetTargetShip(Target);
	LifeSupportStatus->SetTargetShip(Target);
	WeaponStatus->SetTargetShip(Target);
	AFlareSpacecraft* PlayerShip = Target->GetActive();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Is this a civilian ship ?
	WeaponStatus->SetVisibility(Target->IsMilitary() ? EVisibility::Visible : EVisibility::Hidden);
	WeaponContainer->ClearChildren();

	// Update weapon list
	if (PlayerShip && PlayerShip->IsMilitary())
	{
		TArray<FFlareWeaponGroup*>& WeaponGroupList = PlayerShip->GetWeaponsSystem()->GetWeaponGroupList();

		// Add weapon indicators
		for (int32 i = WeaponGroupList.Num() - 1; i >= 0; i--)
		{
			WeaponContainer->AddSlot()
			.AutoHeight()
			[
				SNew(SFlareWeaponStatus)
				.PlayerShip(PlayerShip)
				.TargetWeaponGroupIndex(i)
			];
		}

		// No weapon
		WeaponContainer->AddSlot()
		.AutoHeight()
		[
			SNew(SFlareWeaponStatus)
			.PlayerShip(PlayerShip)
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

	if (TargetShip)
	{
		// Is alive ?
		SetVisibility(TargetShip->GetDamageSystem()->IsAlive() ? EVisibility::Visible : EVisibility::Collapsed);

		// Overheating status
		TimeSinceOverheatChanged += InDeltaTime;
		Temperature = TargetShip->GetDamageSystem()->GetTemperature();
		OverheatTemperature = TargetShip->GetDamageSystem()->GetOverheatTemperature();

		// Alert the player if the ship is near the overheat temperature
		bool NewOverheating = (TargetShip->GetDamageSystem()->GetTemperature() > TargetShip->GetDamageSystem()->GetOverheatTemperature() * 0.95);
		if (NewOverheating != Overheating)
		{
			TimeSinceOverheatChanged = 0;
		}
		Overheating = NewOverheating;

		// Outage status
		TimeSinceOutageChanged += InDeltaTime;
		bool NewPowerOutage = TargetShip->GetDamageSystem()->HasPowerOutage();
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
	if (TargetShip && TargetShip->IsActive() && !MenuManager->GetPC()->UseCockpit && MenuManager->GetPC()->GetGame()->GetActiveSector())
	{
		FText ModeText;
		FText AutopilotText;
		AFlareSpacecraft* ActiveTargetShip = TargetShip->GetActive();

		if (ActiveTargetShip->GetNavigationSystem()->IsDocked())
		{
			ModeText = LOCTEXT("Docked", "Docked");
		}
		else
		{
			ModeText = ActiveTargetShip->GetWeaponsSystem()->GetWeaponModeInfo();
			if (ActiveTargetShip->GetNavigationSystem()->IsAutoPilot())
			{
				AutopilotText = LOCTEXT("AUTOPILOT", " (Autopilot)");
			}
		}

		FText SectorText = FText::Format(LOCTEXT("CurrentSectorFormat", "{0} ({1})"),
			TargetShip->GetGame()->GetActiveSector()->GetSimulatedSector()->GetSectorName(),
			TargetShip->GetGame()->GetActiveSector()->GetSimulatedSector()->GetSectorFriendlynessText(TargetShip->GetCompany()));

		return FText::Format(LOCTEXT("ShipInfoTextFormat", "{0}m/s - {1} {2} - {3}"),
			FText::AsNumber(FMath::RoundToInt(ActiveTargetShip->GetLinearVelocity().Size())),
			ModeText,
			AutopilotText,
			SectorText);
	}

	return FText();
}

FText SFlareHUDMenu::GetLowerInfoText() const
{
	FText Info;

	if (TargetShip && TargetShip->IsActive())
	{
		AFlareSpacecraft* ActiveTargetShip = TargetShip->GetActive();
		UFlareSpacecraftNavigationSystem* Nav = ActiveTargetShip->GetNavigationSystem();
		FFlareShipCommandData Command = Nav->GetCurrentCommand();
		AFlarePlayerController* PC = MenuManager->GetPC();
		
		// Get player threats
		bool Targeted, FiredUpon;
		UFlareSimulatedSpacecraft* Threat;
		PC->GetPlayerShipThreatStatus(Targeted, FiredUpon, Threat);

		// Fired on ?
		if (FiredUpon)
		{
			Info = FText::Format(LOCTEXT("ThreatFiredUponFormat", "UNDER FIRE FROM {0} ({1} - {2})"),
				FText::FromString(Threat->GetImmatriculation().ToString()),
				PC->GetGame()->GetSpacecraftCatalog()->Get(Threat->GetDescription()->Identifier)->Name,
				FText::FromString(Threat->GetCompany()->GetShortName().ToString()));
		}

		// Targeted ?
		else if (Targeted)
		{
			Info = FText::Format(LOCTEXT("ThreatTargetFormat", "TARGETED BY {0} ({1} - {2})"),
				FText::FromString(Threat->GetImmatriculation().ToString()),
				PC->GetGame()->GetSpacecraftCatalog()->Get(Threat->GetDescription()->Identifier)->Name,
				FText::FromString(Threat->GetCompany()->GetShortName().ToString()));
		}

		// Docking info
		else if (Command.Type == EFlareCommandDataType::CDT_Dock)
		{
			AFlareSpacecraft* Target = Command.ActionTarget;
			Info = FText::Format(LOCTEXT("DockingAtFormat", "Docking at {0}"), FText::FromName(Target->GetImmatriculation()));
		}

		// Targetting info
		else
		{
			AFlareSpacecraft* TargetShipPawn = ActiveTargetShip;
			if (TargetShipPawn && TargetShipPawn->GetCurrentTarget())
			{
				Info = FText::Format(LOCTEXT("TargettingFormat", "Targeting {0} ({1})"),
					FText::FromName(TargetShipPawn->GetCurrentTarget()->GetImmatriculation()),
					TargetShipPawn->GetCurrentTarget()->GetParent()->GetCompany()->GetPlayerHostilityText());
			}
		}
	}

	return Info;
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

	if (TargetShip)
	{
		return FText::Format(LOCTEXT("PwBackInFormat", "Power back in {0}..."),
			FText::AsNumber((int32)(TargetShip->GetDamageSystem()->GetPowerOutageDuration()) + 1));
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE
