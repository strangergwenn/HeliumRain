
#include "../../Flare.h"
#include "FlareHUDMenu.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareHUDMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareHUDMenu::Construct(const FArguments& InArgs)
{
	// Data
	OwnerHUD = InArgs._OwnerHUD;
	TargetShip = NULL;
	Overheating = false;
	PresentationFlashTime = 1.0f;
	TimeSinceOverheatChanged = PresentationFlashTime;
	TimeSinceStunChanged = PresentationFlashTime;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());

	// Style
	const FFlareContainerStyle* ContainerStyle = &FFlareStyleSet::Get().GetWidgetStyle<FFlareContainerStyle>("/Style/DefaultContainerStyle");
	
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
		.Padding(FMargin(0, 100))
		[
			SNew(SVerticalBox)

			// Overheating box
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.HAlign(HAlign_Center)
				.BorderImage(&ContainerStyle->BackgroundBrush)
				.BorderBackgroundColor(this, &SFlareHUDMenu::GetOverheatBackgroundColor)
				[
					SNew(SHorizontalBox)

					// Overheating icon
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SImage)
						.Image(FFlareStyleSet::GetIcon("HUD_Temperature"))
						.ColorAndOpacity(this, &SFlareHUDMenu::GetOverheatColor)
					]

					// Overheating text
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.TextStyle(FFlareStyleSet::Get(), "Flare.Title1")
						.Text(LOCTEXT("Overheating", "OVERHEATING"))
						.ColorAndOpacity(this, &SFlareHUDMenu::GetOverheatColor)
					]
				]
			]

			// Stun box
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.HAlign(HAlign_Center)
				.BorderImage(&ContainerStyle->BackgroundBrush)
				.BorderBackgroundColor(this, &SFlareHUDMenu::GetStunBackgroundColor)
				[
					SNew(SHorizontalBox)

					// Stun icon
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SImage)
						.Image(FFlareStyleSet::GetIcon("HUD_Power"))
						.ColorAndOpacity(this, &SFlareHUDMenu::GetStunColor)
					]

					// Stun text
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.TextStyle(FFlareStyleSet::Get(), "Flare.Title1")
						.Text(LOCTEXT("Stunned", "STUNNED"))
						.ColorAndOpacity(this, &SFlareHUDMenu::GetStunColor)
					]
				]
			]
		]

		// Main (bottom) panel
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		[
			SNew(SHorizontalBox)

			// Static container
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(TemperatureStatus, SFlareSubsystemStatus)
					.Subsystem(EFlareSubsystem::SYS_Temperature)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(PowerStatus, SFlareSubsystemStatus)
					.Subsystem(EFlareSubsystem::SYS_Power)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(PropulsionStatus, SFlareSubsystemStatus)
					.Subsystem(EFlareSubsystem::SYS_Propulsion)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(RCSStatus, SFlareSubsystemStatus)
					.Subsystem(EFlareSubsystem::SYS_RCS)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(LifeSupportStatus, SFlareSubsystemStatus)
					.Subsystem(EFlareSubsystem::SYS_LifeSupport)
				]
			]

			// Weapon container
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(WeaponContainer, SHorizontalBox)
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareHUDMenu::SetTargetShip(AFlareShip* Target)
{
	// Set targets
	TargetShip = Target;
	TemperatureStatus->SetTargetShip(Target);
	PowerStatus->SetTargetShip(Target);
	PropulsionStatus->SetTargetShip(Target);
	RCSStatus->SetTargetShip(Target);
	LifeSupportStatus->SetTargetShip(Target);

	// Cleanup old weapon indicators
	TSharedPtr<SFlareSubsystemStatus> Temp;
	WeaponContainer->ClearChildren();

	// Add weapon indicators
	for (int32 i = 0; i < 2; i++)
	{
		WeaponContainer->AddSlot()
			.AutoWidth()
			[
				SAssignNew(Temp, SFlareSubsystemStatus)
				.Subsystem(EFlareSubsystem::SYS_Weapon)
			];
		Temp->SetTargetShip(Target);
	}
	WeaponContainer->SetVisibility(EVisibility::Visible);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareHUDMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (TargetShip)
	{
		// Overheating status
		TimeSinceOverheatChanged += InDeltaTime;
		bool NewOverheating = (TargetShip->GetTemperature() > TargetShip->GetMaxTemperature());
		if (NewOverheating != Overheating)
		{
			TimeSinceOverheatChanged = 0;
		}
		Overheating = NewOverheating;

		// Stun status
		TimeSinceStunChanged += InDeltaTime;
		bool NewStunned = 0; // TODO TargetShip->IsStunned();
		if (NewStunned != Stunned)
		{
			TimeSinceStunChanged = 0;
		}
		Stunned = NewStunned;
	}
}

FSlateColor SFlareHUDMenu::GetOverheatColor() const
{
	FLinearColor Color = FFlareStyleSet::GetHeatColor();
	float Ratio = FMath::Clamp(TimeSinceOverheatChanged / PresentationFlashTime, 0.0f, 1.0f);
	Color.A *= (Overheating ? Ratio : (1 - Ratio));
	return Color;
}

FSlateColor SFlareHUDMenu::GetOverheatBackgroundColor() const
{
	FLinearColor Color = FLinearColor::White;
	Color.A = GetOverheatColor().GetSpecifiedColor().A;
	return Color;
}

FSlateColor SFlareHUDMenu::GetStunColor() const
{
	FLinearColor Color = FFlareStyleSet::GetStunColor();
	float Ratio = FMath::Clamp(TimeSinceStunChanged / PresentationFlashTime, 0.0f, 1.0f);
	Color.A *= (Stunned ? Ratio : (1 - Ratio));
	return Color;
}

FSlateColor SFlareHUDMenu::GetStunBackgroundColor() const
{
	FLinearColor Color = FLinearColor::White;
	Color.A = GetStunColor().GetSpecifiedColor().A;
	return Color;
}


#undef LOCTEXT_NAMESPACE
