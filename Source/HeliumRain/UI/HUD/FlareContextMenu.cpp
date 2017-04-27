
#include "FlareContextMenu.h"
#include "../../Flare.h"
#include "../../Player/FlareHUD.h"
#include "../../Player/FlareMenuManager.h"

#include "Runtime/Engine/Classes/Engine/UserInterfaceSettings.h"
#include "Runtime/Engine/Classes/Engine/RendererSettings.h"

#define LOCTEXT_NAMESPACE "FlareContextMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareContextMenu::Construct(const FArguments& InArgs)
{
	// Data
	PlayerShip = NULL;
	TargetSpacecraft = NULL;
	HUD = InArgs._HUD;
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Structure
	ChildSlot
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Top)
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
		.Padding(this, &SFlareContextMenu::GetContextMenuPosition)
		[
			SNew(SBox)
			.WidthOverride(200)
			.HeightOverride(200)
			[
				SNew(SBorder)
				.BorderImage(FFlareStyleSet::GetIcon("LargeButtonBackground"))
				.BorderBackgroundColor(this, &SFlareContextMenu::GetColor)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SFlareRoundButton)
					.Text(this, &SFlareContextMenu::GetText)
					.Icon(this, &SFlareContextMenu::GetIcon)
					.IconColor(this, &SFlareContextMenu::GetColor)
					.TextColor(this, &SFlareContextMenu::GetColor)
					.HighlightColor(this, &SFlareContextMenu::GetColor)
					.OnClicked(this, &SFlareContextMenu::OnClicked)
				]
			]
		]
	];

	TargetSpacecraft = NULL;
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareContextMenu::SetSpacecraft(AFlareSpacecraft* Target)
{
	TargetSpacecraft = Target;
}

void SFlareContextMenu::OnClicked()
{
	if (TargetSpacecraft && PlayerShip)
	{
		FFlareMenuParameterData Data;
		Data.Spacecraft = TargetSpacecraft->GetParent();
		MenuManager->OpenMenu(EFlareMenu::MENU_Ship, Data);
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
	}
}

FSlateColor SFlareContextMenu::GetColor() const
{
	FLinearColor Color = FLinearColor::White;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	if (TargetSpacecraft &&
		!MenuManager->IsMenuOpen() &&
		!MenuManager->IsSwitchingMenu() &&
		HUD->IsHUDVisible() &&
		!HUD->IsWheelMenuOpen())
	{
		Color.A = Theme.DefaultAlpha;
	}
	else
	{
		Color.A = 0;
	}

	return Color;
}

FMargin SFlareContextMenu::GetContextMenuPosition() const
{
	FVector2D HudLocation = HUD->GetContextMenuLocation();
	FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	float ViewportScale = GetDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass())->GetDPIScaleBasedOnSize(FIntPoint(ViewportSize.X, ViewportSize.Y));

	FVector2D Pos = HudLocation / ViewportScale;
	Pos.X -= 100;
	Pos.Y -= 100;

	return FMargin(Pos.X, Pos.Y, 0, 0);
}

const FSlateBrush* SFlareContextMenu::GetIcon() const
{
	return FFlareStyleSet::GetIcon("DesignatorContextButton");
}

FText SFlareContextMenu::GetText() const
{
	return LOCTEXT("Details", "Details");
}

#undef LOCTEXT_NAMESPACE
