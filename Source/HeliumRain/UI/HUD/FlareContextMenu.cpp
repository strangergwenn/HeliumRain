
#include "../../Flare.h"
#include "../../Player/FlareHUD.h"
#include "../../Player/FlareMenuManager.h"
#include "FlareContextMenu.h"

#include "Runtime/Engine/Classes/Engine/UserInterfaceSettings.h"
#include "Runtime/Engine/Classes/Engine/RendererSettings.h"

#define LOCTEXT_NAMESPACE "FlareContextMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareContextMenu::Construct(const FArguments& InArgs)
{
	// Data
	TargetSpacecraft = NULL;
	HUD = InArgs._HUD;
	MenuManager = InArgs._MenuManager;
	IsTargetting = false;
	PlayerShip = NULL;

	// Structure
	ChildSlot
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Top)
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
		.Padding(this, &SFlareContextMenu::GetContextMenuPosition)
		[
			SNew(SFlareRoundButton)
			.Visibility(this, &SFlareContextMenu::GetButtonVisibility)
			.OnClicked(this, &SFlareContextMenu::OnClicked)
			.Icon(this, &SFlareContextMenu::GetIcon)
			.Text(this, &SFlareContextMenu::GetText)
		]
	];

	Hide();
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareContextMenu::SetSpacecraft(AFlareSpacecraft* Target)
{
	TargetSpacecraft = Target;
}

void SFlareContextMenu::Show()
{
	SetVisibility(EVisibility::Visible);
}

void SFlareContextMenu::Hide()
{
	SetVisibility(EVisibility::Collapsed);
	TargetSpacecraft = NULL;
}

void SFlareContextMenu::OnClicked()
{
	if (TargetSpacecraft && PlayerShip)
	{
		if (IsTargetting)
		{
			UFlareSpacecraftWeaponsSystem* WeaponSystem = PlayerShip->GetWeaponsSystem();
			AFlareSpacecraft* Spacecraft = TargetSpacecraft;
			check(Spacecraft != NULL);

			if (Spacecraft != WeaponSystem->GetActiveWeaponTarget())
			{
				WeaponSystem->SetActiveWeaponTarget(Spacecraft);
			}
			else
			{
				WeaponSystem->SetActiveWeaponTarget(NULL);
			}
		}
		else
		{
			MenuManager->OpenMenuSpacecraft(EFlareMenu::MENU_Ship, TargetSpacecraft->GetParent());
		}
	}
}


/*----------------------------------------------------
	Internal
----------------------------------------------------*/

void SFlareContextMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (IsEnabled() && MenuManager.IsValid())
	{
		PlayerShip = MenuManager->GetPC()->GetShipPawn();

		if (PlayerShip && PlayerShip->IsMilitary() && PlayerShip->GetWeaponsSystem()->GetActiveWeaponType() == EFlareWeaponGroupType::WG_TURRET)
		{
			// Manual targetting is not cool, disabled
			//IsTargetting = true;
		}
		else
		{
			IsTargetting = false;
		}
	}
}

FMargin SFlareContextMenu::GetContextMenuPosition() const
{
	FVector2D HudLocation = HUD->GetContextMenuLocation();
	FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	float ViewportScale = GetDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass())->GetDPIScaleBasedOnSize(FIntPoint(ViewportSize.X, ViewportSize.Y));

	FVector2D Pos = HudLocation / ViewportScale;
	Pos.X -= 48;
	Pos.Y -= 48;

	return FMargin(Pos.X, Pos.Y, 0, 0);
}

EVisibility SFlareContextMenu::GetButtonVisibility() const
{
	if (!IsTargetting || (TargetSpacecraft && TargetSpacecraft->GetParent()->GetCompany()->GetPlayerWarState() <= EFlareHostility::Neutral))
	{
		return EVisibility::Visible;
	}
	else
	{
		return EVisibility::Collapsed;
	}
}

const FSlateBrush* SFlareContextMenu::GetIcon() const
{
	return (IsTargetting ? FFlareStyleSet::GetIcon("TargettingContextButton") : FFlareStyleSet::GetIcon("DesignatorContextButton"));
}

FText SFlareContextMenu::GetText() const
{
	FText Info = LOCTEXT("Inspect", "Inspect");

	if (TargetSpacecraft && PlayerShip)
	{
		if (IsTargetting)
		{
			UFlareSpacecraftWeaponsSystem* WeaponSystem = PlayerShip->GetWeaponsSystem();
			AFlareSpacecraft* Spacecraft = Cast<AFlareSpacecraft>(TargetSpacecraft);
			check(Spacecraft != NULL);

			if (Spacecraft != WeaponSystem->GetActiveWeaponTarget())
			{
				Info = LOCTEXT("Mark", "Mark as target\n {0}");
			}
			else
			{
				Info = LOCTEXT("Clear", "Clear target\n {0}");
			}
		}

		Info = FText::Format(Info, FText::FromName(TargetSpacecraft->GetParent()->GetImmatriculation()));
	}

	return Info;
}

#undef LOCTEXT_NAMESPACE
