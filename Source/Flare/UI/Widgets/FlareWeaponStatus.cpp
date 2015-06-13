
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
	TargetShip = InArgs._TargetShip;
	TargetWeapon = InArgs._TargetWeapon;

	// Setup
	CurrentAlpha = 0;
	FadeInTime = 0.2f;
	FadeOutTime = 1.0f;

	// Content
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	const FSlateBrush* Icon = FFlareStyleSet::GetIcon("Mouse_Nothing_Black");
	if (TargetWeapon)
	{
		Icon = &TargetWeapon->GetDescription()->MeshPreviewBrush;
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

	// Get the animation parameters
	float IsSelected = false;
	float IsSelecting = true;  // TODO : IS THE PLAYER PICKING A WEAPON ?
	if (TargetWeapon)
	{
		// Get the selection state for this weapon
		IsSelected = true; // TODO : IS THIS THE PLAYER'S WEAPON GROUP ?
	}

	// Update animation state
	if (IsSelected || IsSelecting)
	{
		CurrentAlpha += InDeltaTime / FadeInTime;
		if (!IsSelected)
		{
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

	if (TargetWeapon)
	{
		float ComponentHealth = 1.0f; // TODO GET VALUE
		
		FString NameInfo = TargetWeapon->GetDescription()->Name.ToString();
		FString CountInfo = FString::FromInt(1) + "x "; // TODO GET COUNT
		FString AmmoInfo = FString::FromInt(TargetWeapon->GetCurrentAmmo()) + " " + LOCTEXT("Rounds", "rounds").ToString(); // TODO GET GROUP COUNT
		FString HealthInfo = FString::FromInt(100 * ComponentHealth) + "%";

		Text = FText::FromString(CountInfo + NameInfo + "\n" + AmmoInfo/* + "\n" + HealthInfo*/);
	}

	return Text;
}

FSlateColor SFlareWeaponStatus::GetHighlightColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.NeutralColor;
	FLinearColor DamageColor = Theme.EnemyColor;

	// TODO GET HEALTH & LERP
	FLinearColor Color = NormalColor;
	// FLinearColor Color = FMath::Lerp(NormalColor, DamageColor, Health);

	// Update alpha
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
