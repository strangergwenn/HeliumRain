
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
			.HighlightColor(this, &SFlareWeaponStatus::GetIconColor)
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
		]
	];
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareWeaponStatus::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (TargetWeapon)
	{
		// TODO INC/DEC SELECTION ALPHA
	}
	else
	{
	}
}

FText SFlareWeaponStatus::GetText() const
{
	FText Text;

	if (TargetWeapon)
	{
		float ComponentHealth = 1.0f; // TODO GET VALUE
		
		FString NameInfo = TargetWeapon->GetDescription()->Name.ToString();
		FString CountInfo = FString::FromInt(1) + "x "; // TODO GET COUNT
		FString AmmoInfo = FString::FromInt(TargetWeapon->GetCurrentAmmo()) + LOCTEXT("Rounds", " rounds").ToString(); // TODO GET GROUP COUNT
		FString HealthInfo = FString::FromInt(100 * ComponentHealth) + "%";

		Text = FText::FromString(CountInfo + NameInfo + "\n" + AmmoInfo/* + "\n" + HealthInfo*/);
	}

	return Text;
}

FSlateColor SFlareWeaponStatus::GetIconColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.NeutralColor;
	FLinearColor DamageColor = Theme.EnemyColor;

	// TODO GET HEALTH & LERP
	// FLinearColor DamageColor FMath::Lerp(NormalColor, DamageColor, Health);

	// TODO SELECTION ALPHA
	// DamageColor.A *= ...

	return NormalColor;
}

FSlateColor SFlareWeaponStatus::GetTextColor() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor NormalColor = Theme.NeutralColor;
	NormalColor.A = Theme.DefaultAlpha;
	return NormalColor;
}


#undef LOCTEXT_NAMESPACE
