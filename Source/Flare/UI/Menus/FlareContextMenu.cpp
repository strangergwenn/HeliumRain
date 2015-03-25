
#include "../../Flare.h"
#include "../../Player/FlareHUD.h"
#include "FlareContextMenu.h"

#define LOCTEXT_NAMESPACE "FlareContextMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareContextMenu::Construct(const FArguments& InArgs)
{
	// Data
	Visible = false;
	TargetShip = NULL;
	TargetStation = NULL;
	OwnerHUD = InArgs._OwnerHUD;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	const FFlareContainerStyle* ContainerStyle = &FFlareStyleSet::Get().GetWidgetStyle<FFlareContainerStyle>("/Style/ContextMenuButtonStyle");
	const FTextBlockStyle* TextStyle = &FFlareStyleSet::Get().GetWidgetStyle<FTextBlockStyle>("Flare.Title1Inverted");

	// Structure
	ChildSlot
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Top)
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
		.Padding(this, &SFlareContextMenu::GetContextMenuPosition)
		[
			SAssignNew(Container, SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// Legend
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(MinimizedButton, SFlareButton)
					.ContainerStyle(ContainerStyle)
					.OnClicked(this, &SFlareContextMenu::OpenTargetMenu)
					.ButtonStyle(FFlareStyleSet::Get(), "/Style/ContextMenuButton")
				]
			]
		]
	];

	// Legend
	MinimizedButton->GetContainer()->SetContent(
		SNew(STextBlock)
		.Text(this, &SFlareContextMenu::GetLegendText)
		.TextStyle(TextStyle)
	);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareContextMenu::SetStation(IFlareStationInterface* Target)
{
	TargetStation = Target;
	TargetShip = NULL;
}

void SFlareContextMenu::SetShip(IFlareShipInterface* Target)
{
	TargetShip = Target;
	TargetStation = NULL;
}

void SFlareContextMenu::Show()
{
	SetVisibility(EVisibility::Visible);
}

void SFlareContextMenu::Hide()
{
	SetVisibility(EVisibility::Hidden);
}

void SFlareContextMenu::OpenTargetMenu()
{
	if (TargetShip)
	{
		OwnerHUD->OpenMenu(EFlareMenu::MENU_Ship, TargetShip);
	}
	else if (TargetStation)
	{
		OwnerHUD->OpenMenu(EFlareMenu::MENU_Station, TargetStation);
	}
}


/*----------------------------------------------------
	Internal
----------------------------------------------------*/

FText SFlareContextMenu::GetLegendText() const
{
	return FText::FromString("+");
}

FMargin SFlareContextMenu::GetContextMenuPosition() const
{
	FVector2D Pos = OwnerHUD->GetContextMenuLocation();

	Pos.X -= Container->GetDesiredSize().X / 2;
	Pos.Y -= 40;

	return FMargin(Pos.X, Pos.Y, 0, 0);
}

#undef LOCTEXT_NAMESPACE
