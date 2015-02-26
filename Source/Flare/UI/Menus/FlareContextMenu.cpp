
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
	OwnerHUD = InArgs._OwnerHUD;
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	const FFlareContainerStyle* ContainerStyle = &FFlareStyleSet::Get().GetWidgetStyle<FFlareContainerStyle>("/Style/InvertedContainerStyle");
	const FTextBlockStyle* TextStyle = &FFlareStyleSet::Get().GetWidgetStyle<FTextBlockStyle>("Flare.TextInverted");

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
					.OnClicked(this, &SFlareContextMenu::Open)
					.ButtonStyle(FFlareStyleSet::Get(), "/Style/ContextMenuButton")
				]

				// Menu
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(ActionMenu, SFlareTargetActions).Player(PC)
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
	ActionMenu->SetStation(Target);
}

void SFlareContextMenu::SetShip(IFlareShipInterface* Target)
{
	ActionMenu->SetShip(Target);
}

void SFlareContextMenu::Show()
{
	Close();
	Visible = true;
	SetVisibility(EVisibility::Visible);
}

void SFlareContextMenu::Hide()
{
	Close();
	Visible = false;
	SetVisibility(EVisibility::Hidden);
}

void SFlareContextMenu::Open()
{
	MinimizedButton->SetVisibility(EVisibility::Collapsed);
	ActionMenu->Show();
}

void SFlareContextMenu::Close()
{
	MinimizedButton->SetVisibility(EVisibility::Visible);
	ActionMenu->Hide();
}

bool SFlareContextMenu::IsOpen()
{
	return ActionMenu->GetVisibility() == EVisibility::Visible;
}

bool SFlareContextMenu::CanBeHidden()
{
	if (Container.IsValid())
	{
		return Visible && !Container->IsHovered();
	}
	else
	{
		return Visible;
	}
}


/*----------------------------------------------------
	Internal
----------------------------------------------------*/

FText SFlareContextMenu::GetLegendText() const
{
	return FText::FromString(" + " + ActionMenu->GetName());
}

FMargin SFlareContextMenu::GetContextMenuPosition() const
{
	FVector2D Pos = OwnerHUD->GetContextMenuLocation();

	Pos.X -= Container->GetDesiredSize().X / 2;
	Pos.Y -= 25;

	return FMargin(Pos.X, Pos.Y, 0, 0);
}

#undef LOCTEXT_NAMESPACE
