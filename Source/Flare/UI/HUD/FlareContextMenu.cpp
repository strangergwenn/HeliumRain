
#include "../../Flare.h"
#include "../../Player/FlareHUD.h"
#include "../../Player/FlareMenuManager.h"
#include "FlareContextMenu.h"

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
			.OnClicked(this, &SFlareContextMenu::OpenTargetMenu)
			.Icon(FFlareStyleSet::GetIcon("DesignatorContextButton"))
			.Text(this, &SFlareContextMenu::GetText)
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareContextMenu::SetSpacecraft(IFlareSpacecraftInterface* Target)
{
	TargetSpacecraft = Target;
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
	if (TargetSpacecraft)
	{
		MenuManager->OpenMenu(EFlareMenu::MENU_Ship, TargetSpacecraft);
	}
}


/*----------------------------------------------------
	Internal
----------------------------------------------------*/

FMargin SFlareContextMenu::GetContextMenuPosition() const
{
	FVector2D Pos = HUD->GetContextMenuLocation();

	Pos.X -= 48;
	Pos.Y -= 48;

	return FMargin(Pos.X, Pos.Y, 0, 0);
}

FText SFlareContextMenu::GetText() const
{
	FText Result;
	AFlareSpacecraft* Candidate = NULL;


	if (GetVisibility() == EVisibility::Visible)
	{
		if (TargetSpacecraft)
		{
			Candidate = Cast<AFlareSpacecraft>(TargetSpacecraft);
		}
		if (Candidate)
		{
			Result = FText::FromString(Candidate->GetImmatriculation().ToString());
		}
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE
