
#include "../../Flare.h"
#include "FlareTradeRouteMenu.h"

#include "../../Game/FlareGame.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#define LOCTEXT_NAMESPACE "FlareTradeRouteMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareTradeRouteMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	AFlarePlayerController* PC = MenuManager->GetPC();

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Padding(Theme.ContentPadding)
		[
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SImage).Image(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_TradeRoute))
			]

			// Title
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(Theme.ContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TitleFont)
				.Text(LOCTEXT("TradeRouteInfo", "TRADE ROUTE INFO"))
			]

			// Close
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(Theme.TitleButtonPadding)
			.AutoWidth()
			[
				SNew(SFlareRoundButton)
				.Text(LOCTEXT("GoOrbit", "Orbital map"))
				.HelpText(LOCTEXT("GoOrbitInfo", "Exit the sector menu and go back to the orbital map"))
				.Icon(AFlareMenuManager::GetMenuIcon(EFlareMenu::MENU_Orbit, true))
				.OnClicked(this, &SFlareTradeRouteMenu::OnBackClicked)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(200, 20))
		[
			SNew(SImage).Image(&Theme.SeparatorBrush)
		]

		// UI container
		+ SVerticalBox::Slot()
		.Padding(Theme.ContentPadding)
		.HAlign(HAlign_Center)
		[
			SNew(SVerticalBox)

			// Header
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// Info
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Top)
				[
					SNew(SVerticalBox)

					// Trade route name
					+ SVerticalBox::Slot()
					.Padding(Theme.TitlePadding)
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(this, &SFlareTradeRouteMenu::GetTradeRouteName)
						.TextStyle(&Theme.SubTitleFont)
                    ]

                    // Change name
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Left)
					[
						SNew(SFlareButton)
						.Width(8)
						.Text(LOCTEXT("ChangeName", "Change route name"))
                        .HelpText(LOCTEXT("ChangeNameInfo", "Change the route name"))
						.Icon(FFlareStyleSet::GetIcon("ChangeRouteName"))
						.OnClicked(this, &SFlareTradeRouteMenu::OnChangeRouteNameClicked)
                        .Visibility(this, &SFlareTradeRouteMenu::GetShowingRouteNameVisibility)
					]

                    // Trade route name
                    + SVerticalBox::Slot()
                    .Padding(Theme.TitlePadding)
                    .AutoHeight()
                    [
                        SAssignNew(EditRouteName, SEditableText)
                        .Style(&Theme.TextInputStyle)
                        .Visibility(this, &SFlareTradeRouteMenu::GetEditingRouteNameVisibility)
                    ]

                    // Confirm name
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(Theme.ContentPadding)
                    .HAlign(HAlign_Left)
                    [
                        SNew(SFlareButton)
                        .Width(8)
                        .Text(LOCTEXT("ConfirmChangeName", "Confirm"))
                        .HelpText(LOCTEXT("ConfirmChangeNameInfo", "Confirm change the route name"))
                        .Icon(FFlareStyleSet::GetIcon("ConfirmChangeRouteName"))
                        .OnClicked(this, &SFlareTradeRouteMenu::OnConfirmChangeRouteNameClicked)
                        .Visibility(this, &SFlareTradeRouteMenu::GetEditingRouteNameVisibility)
                    ]

                    // Cancel change name
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(Theme.ContentPadding)
                    .HAlign(HAlign_Left)
                    [
                        SNew(SFlareButton)
                        .Width(8)
                        .Text(LOCTEXT("CancelChangeName", "Cancel"))
                        .HelpText(LOCTEXT("CancelChangeNameInfo", "Cancel name change"))
                        .Icon(FFlareStyleSet::GetIcon("CancelChangeRouteName"))
                        .OnClicked(this, &SFlareTradeRouteMenu::OnCancelChangeRouteNameClicked)
                        .Visibility(this, &SFlareTradeRouteMenu::GetEditingRouteNameVisibility)
                    ]

                    // Sector selection
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(Theme.ContentPadding)
                    [
                        SAssignNew(SectorSelector, SComboBox<UFlareSimulatedSector*>)
                        .OptionsSource(&SectorList)
                        .OnGenerateWidget(this, &SFlareTradeRouteMenu::OnGenerateSectorComboLine)
                        .OnSelectionChanged(this, &SFlareTradeRouteMenu::OnSectorComboLineSelectionChanged)
                        .ComboBoxStyle(&Theme.ComboBoxStyle)
                        .ForegroundColor(FLinearColor::White)
                        .Visibility(this, &SFlareTradeRouteMenu::GetAddSectorVisibility)
                        [
                            SNew(STextBlock)
                            .Text(this, &SFlareTradeRouteMenu::OnGetCurrentSectorComboLine)
                            .TextStyle(&Theme.TextFont)
                        ]
                    ]

                    // Add sector button
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(Theme.ContentPadding)
                    .HAlign(HAlign_Left)
                    [
                        SNew(SFlareButton)
                        .Width(8)
                        .Text(LOCTEXT("AddSector", "Add a sector"))
                        .HelpText(LOCTEXT("AddSectorInfo", "Add a sector to the trade route"))
                        .OnClicked(this, &SFlareTradeRouteMenu::OnAddSectorClicked)
                        .Visibility(this, &SFlareTradeRouteMenu::GetAddSectorVisibility)
                    ]

                    // Trade route sector list
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .HAlign(HAlign_Left)
                    [
                        SAssignNew(TradeSectorList, SVerticalBox)
                    ]
                ]
             ]

			// Content block
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			[
				SNew(SScrollBox)
				.Style(&Theme.ScrollBoxStyle)
				.ScrollBarStyle(&Theme.ScrollBarStyle)

				+ SScrollBox::Slot()
				[
					SNew(SHorizontalBox)

				]
			]
		]
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareTradeRouteMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Hidden);
}

void SFlareTradeRouteMenu::Enter(UFlareTradeRoute* TradeRoute)
{
	FLOG("SFlareTradeMenu::Enter");

    IsEditingName = false;
	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	TargetTradeRoute = TradeRoute;
	AFlarePlayerController* PC = MenuManager->GetPC();

    GenerateSectorList();
}

void SFlareTradeRouteMenu::Exit()
{
	SetEnabled(false);
	TargetTradeRoute = NULL;
    TradeSectorList->ClearChildren();

	SetVisibility(EVisibility::Hidden);
}

void SFlareTradeRouteMenu::GenerateSectorList()
{
    SectorList.Empty();
    TradeSectorList->ClearChildren();

    if (TargetTradeRoute)
    {
        TArray<UFlareSimulatedSector*>& VisitedSectors = MenuManager->GetGame()->GetPC()->GetCompany()->GetVisitedSectors();
        for (int SectorIndex = 0; SectorIndex < VisitedSectors.Num(); SectorIndex++)
        {
            if (!TargetTradeRoute->IsVisiting(VisitedSectors[SectorIndex]))
            {
                SectorList.Add(VisitedSectors[SectorIndex]);
            }
        }

        const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();


        TArray<FFlareTradeRouteSectorSave>& Sectors = TargetTradeRoute->GetSectors();
        for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
        {
            FFlareTradeRouteSectorSave* SectorOrders = &Sectors[SectorIndex];
            UFlareSimulatedSector* Sector = MenuManager->GetGame()->GetGameWorld()->FindSector(SectorOrders->SectorIdentifier);

            // TODO Trade route sector info
            TradeSectorList->AddSlot()
            .AutoHeight()
            .HAlign(HAlign_Right)
            [
                SNew(SHorizontalBox)

                // Sector info
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(Theme.ContentPadding)
                [
                    SNew(SBox)
                    .HAlign(HAlign_Left)
                    .VAlign(VAlign_Center)
                    [
                        SNew(STextBlock)
                        .TextStyle(&Theme.TextFont)
                        .Text(Sector->GetSectorName())
                    ]
                ]

               /* // Inspect
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SFlareButton)
                    .OnClicked(this, &SFlareCompanyMenu::OnInspectTradeRouteClicked, TradeRoute)
                    .Text(FText(LOCTEXT("Inspect", "Inspect")))
                ]*/

            ];
        }
    }
}

/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareTradeRouteMenu::OnBackClicked()
{
	MenuManager->Back();
}

void SFlareTradeRouteMenu::OnChangeRouteNameClicked()
{
    EditRouteName->SetText(GetTradeRouteName());
    IsEditingName = true;
}

void SFlareTradeRouteMenu::OnCancelChangeRouteNameClicked()
{
    IsEditingName = false;
}

void SFlareTradeRouteMenu::OnConfirmChangeRouteNameClicked()
{
    if (TargetTradeRoute)
    {
        TargetTradeRoute->SetTradeRouteName(EditRouteName->GetText());
    }
    IsEditingName = false;
}

EVisibility SFlareTradeRouteMenu::GetShowingRouteNameVisibility() const
{
    return IsEditingName ? EVisibility::Collapsed : EVisibility::Visible;
}

EVisibility SFlareTradeRouteMenu::GetEditingRouteNameVisibility() const
{
    return IsEditingName ? EVisibility::Visible : EVisibility::Collapsed;
}

/** Get a button text */
FText SFlareTradeRouteMenu::GetTradeRouteName() const
{
	FText Result;

	if (TargetTradeRoute)
	{
		Result = TargetTradeRoute->GetTradeRouteName();
	}

	return Result;
}


TSharedRef<SWidget> SFlareTradeRouteMenu::OnGenerateSectorComboLine(UFlareSimulatedSector* Item)
{
    const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

    return SNew(STextBlock)
    .Text(Item->GetSectorName())
    .TextStyle(&Theme.TextFont);
}

void SFlareTradeRouteMenu::OnSectorComboLineSelectionChanged(UFlareSimulatedSector* Item, ESelectInfo::Type SelectInfo)
{
}

FText SFlareTradeRouteMenu::OnGetCurrentSectorComboLine() const
{
    UFlareSimulatedSector* Item = SectorSelector->GetSelectedItem();
    return Item ? Item->GetSectorName() : LOCTEXT("Select", "Select a sector");
}

void SFlareTradeRouteMenu::OnAddSectorClicked()
{
    UFlareSimulatedSector* Item = SectorSelector->GetSelectedItem();
    if (Item)
    {
        TargetTradeRoute->AddSector(Item);
        GenerateSectorList();
        SectorSelector->SetSelectedItem(NULL);
    }
}

EVisibility SFlareTradeRouteMenu::GetAddSectorVisibility() const
{
    return SectorList.Num() > 0 ? EVisibility::Visible : EVisibility::Collapsed;
}




#undef LOCTEXT_NAMESPACE

