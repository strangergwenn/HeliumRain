
#include "../../Flare.h"
#include "FlareColorPanel.h"
#include "../../Game/FlareGame.h"
#include "../../Player/FlareHUD.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareColorPanel"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareColorPanel::Construct(const FArguments& InArgs)
{
	// Data
	OwnerHUD = InArgs._OwnerHUD;
	AFlareGame* Game = Cast<AFlareGame>(OwnerHUD->GetWorld()->GetAuthGameMode());
	UFlareCustomizationCatalog* CustomizationCatalog = Game->GetCustomizationCatalog();
	UFlareSpacecraftComponentsCatalog* ShipPartsCatalog = Game->GetShipPartsCatalog();
	
	// Layout
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SAssignNew(PickerList, SHorizontalBox)
		
		// Pattern picker
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SAssignNew(PatternPicker, SFlareDropList)
			.OnItemPicked(this, &SFlareColorPanel::OnPatternPicked)
		]
		
		// Base paint picker
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SAssignNew(BasePaintColorPicker, SFlareDropList)
			.OnItemPicked(this, &SFlareColorPanel::OnBasePaintColorPicked)
			.LineSize(3)
			.ItemWidth(1)
			.ItemHeight(1)
		]
		
		// Paint picker
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SAssignNew(PaintColorPicker, SFlareDropList)
			.OnItemPicked(this, &SFlareColorPanel::OnPaintColorPicked)
			.LineSize(3)
			.ItemWidth(1)
			.ItemHeight(1)
		]
		
		// Overlay picker
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SAssignNew(OverlayColorPicker, SFlareDropList)
			.OnItemPicked(this, &SFlareColorPanel::OnOverlayColorPicked)
			.LineSize(3)
			.ItemWidth(1)
			.ItemHeight(1)
		]
		
		// Light picker
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SAssignNew(LightColorPicker, SFlareDropList)
			.OnItemPicked(this, &SFlareColorPanel::OnLightColorPicked)
			.LineSize(3)
			.ItemWidth(1)
			.ItemHeight(1)
		]
	];

	// Fill patterns
	for (int i = 0; i < CustomizationCatalog->GetPatternCount(); i++)
	{
		PatternPicker->AddItem(SNew(SImage).Image(CustomizationCatalog->GetPatternBrush(i)));
	}

	// Fill colors
	for (int i = 0; i < CustomizationCatalog->GetColorCount(); i++)
	{
		BasePaintColorPicker->AddItem(SNew(SColorBlock).Color(CustomizationCatalog->GetColor(i)));
		PaintColorPicker->AddItem(SNew(SColorBlock).Color(CustomizationCatalog->GetColor(i)));
		OverlayColorPicker->AddItem(SNew(SColorBlock).Color(CustomizationCatalog->GetColor(i)));
		LightColorPicker->AddItem(SNew(SColorBlock).Color(CustomizationCatalog->GetColor(i)));
	}

	// Setup initial data
	BasePaintColorPicker->SetSelectedIndex(0);
	PaintColorPicker->SetSelectedIndex(0);
	OverlayColorPicker->SetSelectedIndex(0);
	LightColorPicker->SetSelectedIndex(0);
	PatternPicker->SetSelectedIndex(0);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareColorPanel::Setup(FFlarePlayerSave& PlayerData)
{
	AFlareGame* Game = Cast<AFlareGame>(OwnerHUD->GetWorld()->GetAuthGameMode());
	UFlareCompany* Company = Game->FindCompany(PlayerData.CompanyIdentifier);
	if (Company)
	{
		BasePaintColorPicker->SetSelectedIndex(Company->GetBasePaintColorIndex());
		PaintColorPicker->SetSelectedIndex(Company->GetPaintColorIndex());
		OverlayColorPicker->SetSelectedIndex(Company->GetOverlayColorIndex());
		LightColorPicker->SetSelectedIndex(Company->GetLightColorIndex());
		PatternPicker->SetSelectedIndex(Company->GetPatternIndex());
	}
}

void SFlareColorPanel::OnBasePaintColorPicked(int32 Index)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
		FLOGV("SFlareColorPanel::OnBasePaintColorPicked %d", Index);
		PC->GetCompany()->SetBasePaintColorIndex(Index);
		PC->GetMenuPawn()->UpdateCustomization();
	}
}

void SFlareColorPanel::OnPaintColorPicked(int32 Index)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
		FLOGV("SFlareColorPanel::OnPaintColorPicked %d", Index);
		PC->GetCompany()->SetPaintColorIndex(Index);
		PC->GetMenuPawn()->UpdateCustomization();
	}
}

void SFlareColorPanel::OnOverlayColorPicked(int32 Index)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
		FLOGV("SFlareColorPanel::OnOverlayColorPicked %d", Index);
		PC->GetCompany()->SetOverlayColorIndex(Index);
		PC->GetMenuPawn()->UpdateCustomization();
	}
}

void SFlareColorPanel::OnLightColorPicked(int32 Index)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
		FLOGV("SFlareColorPanel::OnLightColorPicked %d", Index);
		PC->GetCompany()->SetLightColorIndex(Index);
		PC->GetMenuPawn()->UpdateCustomization();
	}
}

void SFlareColorPanel::OnPatternPicked(int32 Index)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
		FLOGV("SFlareColorPanel::OnPatternPicked %d", Index);
		PC->GetCompany()->SetPatternIndex(Index);
		PC->GetMenuPawn()->UpdateCustomization();
	}
}

#undef LOCTEXT_NAMESPACE
