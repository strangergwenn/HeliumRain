
#include "../../Flare.h"
#include "FlareSubsystemStatus.h"
#include "../Components/FlareButton.h"

#define LOCTEXT_NAMESPACE "FlareSubsystemStatus"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareSubsystemStatus::Construct(const FArguments& InArgs)
{
	// Data
	TargetShip = NULL;
	SubsystemType = InArgs._Subsystem;
	const FFlareButtonStyle* ButtonStyle = &FFlareStyleSet::Get().GetWidgetStyle<FFlareButtonStyle>("/Style/ContextMenuButton");
	const FFlareContainerStyle* ContainerStyle = &FFlareStyleSet::Get().GetWidgetStyle<FFlareContainerStyle>("/Style/DefaultContainerStyle");

	// Structure
	ChildSlot
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Center)
	[
		// Content box
		SNew(SBox)
		.WidthOverride(ButtonStyle->Width)
		.HeightOverride(ButtonStyle->Height)
		.Padding(FMargin(0))
		[
			// Button background
			SNew(SBorder)
			.Padding(ContainerStyle->BorderPadding)
			.BorderImage(&ContainerStyle->BackgroundBrush)
			[
				SNew(SVerticalBox)

				// Status string
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(this, &SFlareSubsystemStatus::GetStatusText)
					.TextStyle(FFlareStyleSet::Get(), "Flare.Text")
				]

				// Icon
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SImage)
					.Image(this, &SFlareSubsystemStatus::GetIcon)
					.ColorAndOpacity(this, &SFlareSubsystemStatus::GetIconColor)
				]

				// Subsystem type
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(this, &SFlareSubsystemStatus::GetTypeText)
					.TextStyle(FFlareStyleSet::Get(), "Flare.Text")
				]
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareSubsystemStatus::SetTargetShip(AFlareShip* Target)
{
	TargetShip = Target;
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

const FSlateBrush* SFlareSubsystemStatus::GetIcon() const
{
	return FFlareStyleSet::GetIcon("Armor");
}

FSlateColor SFlareSubsystemStatus::GetIconColor() const
{
	return FLinearColor::Green;
}

FText SFlareSubsystemStatus::GetStatusText() const
{
	return LOCTEXT("aaa", "70%");
}

FText SFlareSubsystemStatus::GetTypeText() const
{
	return LOCTEXT("BBB", "TYPE");
}


#undef LOCTEXT_NAMESPACE
