
#include "../../Flare.h"
#include "FlareWeaponStatus.h"
#include "../Components/FlareRoundButton.h"
#include "../../Spacecrafts/FlareWeapon.h"
#include "../../Spacecrafts/FlareSpacecraft.h"
#include "../../Player/FlarePlayerController.h"

#define LOCTEXT_NAMESPACE "FlareWeaponStatus"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareWeaponStatus::Construct(const FArguments& InArgs)
{
	// Args
	PlayerShip = InArgs._PlayerShip;
	TargetWeaponGroupIndex = InArgs._TargetWeaponGroupIndex;

	// Setup
	ComponentHealth = 1.0;
	CurrentAlpha = 0;
	FadeInTime = 0.5f;
	FadeOutTime = 1.0f;

	// Content
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	const FSlateBrush* Icon = FFlareStyleSet::GetIcon("Mouse_Nothing_Black");
	if (PlayerShip && TargetWeaponGroupIndex >= 0)
	{
		TargetWeaponGroup = PlayerShip->GetWeaponsSystem()->GetWeaponGroup(TargetWeaponGroupIndex);
		Icon = &TargetWeaponGroup->Description->MeshPreviewBrush;
	}
	else
	{
		TargetWeaponGroup = NULL;
	}
	
	// Structure
	ChildSlot
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Left)
	.Padding(Theme.ContentPadding)
	[
		SNew(SHorizontalBox)

		// Button
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SFlareRoundButton)
			.ShowText(false)
			.HelpText(LOCTEXT("WeaponGroupInfo", "You can switch weapon groups by using the mouse wheel"))
			.InvertedBackground(true)
			.Icon(Icon)
			.IconColor(this, &SFlareWeaponStatus::GetIconColor)
			.HighlightColor(this, &SFlareWeaponStatus::GetHighlightColor)
		]
		
		// Text
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(Theme.ContentPadding)
		[
			SNew(STextBlock)
			.TextStyle(&Theme.SmallFont)
			.Text(this, &SFlareWeaponStatus::GetText)
			.ColorAndOpacity(this, &SFlareWeaponStatus::GetTextColor)
			.ShadowColorAndOpacity(this, &SFlareWeaponStatus::GetShadowColor)
		]
	];
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareWeaponStatus::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Get the selection state for this weapon
	float IsSelected = false;
	float IsSelecting = false;
	if (PlayerShip)
	{
		IsSelected = (PlayerShip->GetWeaponsSystem()->GetActiveWeaponGroupIndex() == TargetWeaponGroupIndex);
		AFlarePlayerController* PC = PlayerShip->GetPC();
		if (PC)
		{
			IsSelecting = PC->IsSelectingWeapon();
		}
	}

	// Health
	if (PlayerShip && TargetWeaponGroupIndex >= 0)
	{
		ComponentHealth = PlayerShip->GetDamageSystem()->GetWeaponGroupHealth(TargetWeaponGroupIndex);
	}
	else
	{
		ComponentHealth = 1.0;
	}

	// Update animation state
	if (IsSelected)
	{
		CurrentAlpha += InDeltaTime / FadeInTime;
	}
	else if (IsSelecting)
	{
		if (CurrentAlpha >= 0.5f)
		{
			CurrentAlpha -= InDeltaTime / FadeOutTime;
		}
		else
		{
			CurrentAlpha += InDeltaTime / FadeInTime;
			CurrentAlpha = FMath::Clamp(CurrentAlpha, 0.0f, 0.5f);
		}
	}
	else
	{
		CurrentAlpha -= InDeltaTime / FadeOutTime;
	}
	CurrentAlpha = FMath::Clamp(CurrentAlpha, 0.0f, 1.0f);
}

FText SFlareWeaponStatus::GetText() const
{
	FText Text;

	if (TargetWeaponGroup)
	{
		// Get ammo count
		int32 RemainingAmmo = 0;
		for (int32 i = 0; i < TargetWeaponGroup->Weapons.Num(); i++)
		{
			if (TargetWeaponGroup->Weapons[i]->GetDamageRatio() <= 0.)
			{
				// Don't count ammo from destroyed components
				continue;
			}
			RemainingAmmo += TargetWeaponGroup->Weapons[i]->GetCurrentAmmo();
		}

		// Final string
		Text = FText::Format(LOCTEXT("WeaponInfoFormat", "{0}x {1}\n{2}\n{3}%"),
			FText::AsNumber(TargetWeaponGroup->Weapons.Num()), TargetWeaponGroup->Description->Name,
			FText::Format(LOCTEXT("Rounds", "{0} rounds"), FText::AsNumber(RemainingAmmo)),
			FText::AsNumber(100 * ComponentHealth));
	}

	return Text;
}

FSlateColor SFlareWeaponStatus::GetHighlightColor() const
{
	FLinearColor Color = FFlareStyleSet::GetHealthColor(ComponentHealth, true);
	Color.A *= CurrentAlpha;
	return Color;
}

FSlateColor SFlareWeaponStatus::GetIconColor() const
{
	FLinearColor NormalColor = FLinearColor::White;
	NormalColor.A *= CurrentAlpha;
	return NormalColor;
}

FSlateColor SFlareWeaponStatus::GetTextColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.NeutralColor;
	NormalColor.A *= Theme.DefaultAlpha * CurrentAlpha;
	return NormalColor;
}

FLinearColor SFlareWeaponStatus::GetShadowColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.SmallFont.ShadowColorAndOpacity;
	NormalColor.A *= Theme.DefaultAlpha * CurrentAlpha;
	return NormalColor;
}


#undef LOCTEXT_NAMESPACE
