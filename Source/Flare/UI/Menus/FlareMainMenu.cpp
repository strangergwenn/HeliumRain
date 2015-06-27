
#include "../../Flare.h"
#include "FlareMainMenu.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuPawn.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareMainMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareMainMenu::Construct(const FArguments& InArgs)
{
	// Data
	OwnerHUD = InArgs._OwnerHUD;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SFlareButton)
		.OnClicked(this, &SFlareMainMenu::OnStartGame)
	];

}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareMainMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}

void SFlareMainMenu::Enter()
{
	FLOG("SFlareMainMenu::Enter");
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
	}
}

void SFlareMainMenu::Exit()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareMainMenu::OnStartGame() const
{

}


#undef LOCTEXT_NAMESPACE

