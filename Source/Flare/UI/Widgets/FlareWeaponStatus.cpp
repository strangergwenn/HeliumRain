
#include "../../Flare.h"
#include "FlareWeaponStatus.h"
#include "../Components/FlareRoundButton.h"
#include "../../Spacecrafts/FlareWeapon.h"
#include "../../Spacecrafts/FlareSpacecraft.h"

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
		// Generic info
		FString NameInfo = TargetWeaponGroup->Description->Name.ToString();
		FString CountInfo = FString::FromInt(TargetWeaponGroup->Weapons.Num()) + "x ";
		FString HealthInfo = FString::FromInt(100 * ComponentHealth) + "%";

		// Get ammo count
		int32 RemainingAmmo = 0;
		for (int32 i = 0; i < TargetWeaponGroup->Weapons.Num(); i++)
		{
			RemainingAmmo += TargetWeaponGroup->Weapons[i]->GetCurrentAmmo();
		}
		FString AmmoInfo = FString::FromInt(RemainingAmmo) + " " + LOCTEXT("Rounds", "rounds").ToString();

		// Final string
		Text = FText::FromString(CountInfo + NameInfo + "\n" + AmmoInfo + "\n" + HealthInfo);
	}

	return Text;
}

FSlateColor SFlareWeaponStatus::GetHighlightColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.NeutralColor;
	FLinearColor DamageColor = Theme.EnemyColor;

	// Update alpha
	FLinearColor Color = FMath::Lerp(DamageColor, NormalColor, ComponentHealth);
	Color.A *= Theme.DefaultAlpha * CurrentAlpha;
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
	FLinearColor NormalColor = Theme.InvertedColor;
	NormalColor.A *= Theme.DefaultAlpha * CurrentAlpha;
	return NormalColor;
}


#undef LOCTEXT_NAMESPACE
