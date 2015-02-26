
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
	UFlareShipPartsCatalog* ShipPartsCatalog = Game->GetShipPartsCatalog();
	
	// Layout
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SAssignNew(PickerList, SHorizontalBox)
		
		// Pattern picker
		+ SHorizontalBox::Slot().AutoWidth()
		.Padding(FMargin(5, 0))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 5)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PaintSchemePattern", "PATTERN"))
				.TextStyle(FFlareStyleSet::Get(), "Flare.Title3")
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SAssignNew(PatternPicker, SFlareDropList)
				.OnItemPicked(this, &SFlareColorPanel::OnPatternPicked)
				.ItemStyle(FFlareStyleSet::Get(), "/Style/PatternButton")
				.HeaderStyle(FFlareStyleSet::Get(), "/Style/PatternButton")
			]
		]
		
		// Paint picker
		+ SHorizontalBox::Slot().AutoWidth()
		.Padding(FMargin(5, 0))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 5)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PaintSchemeColor", "OVERLAY"))
				.TextStyle(FFlareStyleSet::Get(), "Flare.Title3")
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SAssignNew(PaintColorPicker, SFlareDropList)
				.OnItemPicked(this, &SFlareColorPanel::OnPaintColorPicked)
				.ItemStyle(FFlareStyleSet::Get(), "/Style/ColorButton")
				.HeaderStyle(FFlareStyleSet::Get(), "/Style/PatternButton")
				.LineSize(4)
			]
		]
		
		// Light picker
		+ SHorizontalBox::Slot().AutoWidth()
		.Padding(FMargin(5, 0))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 5)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PaintSchemeEngine", "ENGINES"))
				.TextStyle(FFlareStyleSet::Get(), "Flare.Title3")
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SAssignNew(LightColorPicker, SFlareDropList)
				.OnItemPicked(this, &SFlareColorPanel::OnLightColorPicked)
				.ItemStyle(FFlareStyleSet::Get(), "/Style/ColorButton")
				.HeaderStyle(FFlareStyleSet::Get(), "/Style/PatternButton")
				.LineSize(4)
			]
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
		PaintColorPicker->AddItem(SNew(SColorBlock).Color(CustomizationCatalog->GetColor(i)));
		LightColorPicker->AddItem(SNew(SColorBlock).Color(CustomizationCatalog->GetColor(i)));
	}

	// Setup initial data
	PaintColorPicker->SetSelectedIndex(0);
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
		PaintColorPicker->SetSelectedIndex(Company->GetPaintColorIndex());
		LightColorPicker->SetSelectedIndex(Company->GetLightColorIndex());
		PatternPicker->SetSelectedIndex(Company->GetPatternIndex());
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

void SFlareColorPanel::OnLightColorPicked(int32 Index)
{
	AFlarePlayerController* PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	if (PC)
	{
		FLOGV("SFlareColorPanel::OnLightColorPicked %d", Index);
		PC->GetCompany()->SetLightColorIndex(Index);
		PC->GetCompany()->SetEngineColorIndex(Index);
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
