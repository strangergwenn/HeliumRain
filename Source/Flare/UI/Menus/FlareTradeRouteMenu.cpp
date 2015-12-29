
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

    for (int ResourceIndex = 0; ResourceIndex < PC->GetGame()->GetResourceCatalog()->Resources.Num(); ResourceIndex++)
    {
        ResourceList.Add(PC->GetGame()->GetResourceCatalog()->Resources[ResourceIndex]);
    }
    //TODO need duplication ?

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

                    SNew(SVerticalBox)

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

                    // Resource selection
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(Theme.ContentPadding)
                    [
                        SAssignNew(ResourceSelector, SComboBox<UFlareResourceCatalogEntry*>)
                        .OptionsSource(&PC->GetGame()->GetResourceCatalog()->Resources)
                        .OnGenerateWidget(this, &SFlareTradeRouteMenu::OnGenerateResourceComboLine)
                        .OnSelectionChanged(this, &SFlareTradeRouteMenu::OnResourceComboLineSelectionChanged)
                        .ComboBoxStyle(&Theme.ComboBoxStyle)
                        .ForegroundColor(FLinearColor::White)
                        .Visibility(this, &SFlareTradeRouteMenu::GetResourceSelectorVisibility)
                        [
                            SNew(STextBlock)
                            .Text(this, &SFlareTradeRouteMenu::OnGetCurrentResourceComboLine)
                            .TextStyle(&Theme.TextFont)
                        ]
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

            TSharedPtr<SVerticalBox>                        LoadResourceList;
            TSharedPtr<SVerticalBox>                        UnloadResourceList;


            // TODO Trade route sector info
            TradeSectorList->AddSlot()
            .AutoHeight()
            .HAlign(HAlign_Right)
            [
                SNew(SVerticalBox)

                // Sector info
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
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

                // Add load resource
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                [
                    SNew(SFlareButton)
                    .OnClicked(this, &SFlareTradeRouteMenu::OnLoadResourceClicked, Sector)
                    .Text(FText(LOCTEXT("LoadResource", "Load")))
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                [
                        SAssignNew(LoadResourceList, SVerticalBox)
                ]

                // Unload load resource
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                [
                    SNew(SFlareButton)
                    .OnClicked(this, &SFlareTradeRouteMenu::OnUnloadResourceClicked, Sector)
                    .Text(FText(LOCTEXT("UnloadResource", "Unload")))
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Left)
                [
                        SAssignNew(UnloadResourceList, SVerticalBox)
                ]
            ];

            // Fill resource lists

            for (int ResourceIndex = 0; ResourceIndex < SectorOrders->ResourcesToLoad.Num(); ResourceIndex++)
            {
                FFlareCargoSave* LoadedResource = &SectorOrders->ResourcesToLoad[ResourceIndex];
                FFlareResourceDescription* Resource = MenuManager->GetGame()->GetResourceCatalog()->Get(LoadedResource->ResourceIdentifier);

                FText LoadLimits;
                if(LoadedResource->Quantity > 0)
                {
                    LoadLimits = FText::Format(LOCTEXT("LoadLimits", "Left {0} units in the sector."), FText::AsNumber(LoadedResource->Quantity));
                }
                else
                {
                    LoadLimits = FText(LOCTEXT("NoLoadLimits", "Load all."));
                }

                LoadResourceList->AddSlot()
                .AutoHeight()
                .HAlign(HAlign_Right)
                [
                    SNew(SHorizontalBox)

                    // Resource name
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
                            .Text(Resource->Name)
                        ]
                    ]

                    // Limits
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
                            .Text(LoadLimits)
                        ]
                    ]

                    // Limit decrease
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SFlareButton)
                        .OnClicked(this, &SFlareTradeRouteMenu::OnDecreaseLoadLimitClicked, Sector, Resource)
                        .Text(FText::FromString(TEXT("-")))
                        .Transparent(true)
                        .Width(1)
                    ]

                    // Limit increase
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SFlareButton)
                        .OnClicked(this, &SFlareTradeRouteMenu::OnIncreaseLoadLimitClicked, Sector, Resource)
                        .Text(FText::FromString(TEXT("+")))
                        .Transparent(true)
                        .Width(1)
                    ]

                    // Clear resource load
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SFlareButton)
                        .OnClicked(this, &SFlareTradeRouteMenu::OnClearLoadResourceClicked, Sector, Resource)
                        .Text(FText())
                        .HelpText(LOCTEXT("ClearLoadResource", "Stop loading this resource"))
                        .Icon(FFlareStyleSet::GetIcon("Stop"))
                        .Transparent(true)
                        .Width(1)
                    ]
                ];
            }

            for (int ResourceIndex = 0; ResourceIndex < SectorOrders->ResourcesToUnload.Num(); ResourceIndex++)
            {
                FFlareCargoSave* UnloadedResource = &SectorOrders->ResourcesToUnload[ResourceIndex];
                FFlareResourceDescription* Resource = MenuManager->GetGame()->GetResourceCatalog()->Get(UnloadedResource->ResourceIdentifier);

                FText UnloadLimits;
                if(UnloadedResource->Quantity > 0)
                {
                    UnloadLimits = FText::Format(LOCTEXT("UnloadLimits", "Unload max {0} units in the sector."), FText::AsNumber(UnloadedResource->Quantity));
                }
                else
                {
                    UnloadLimits = FText(LOCTEXT("UnloadLimits", "Unload all."));
                }

                UnloadResourceList->AddSlot()
                .AutoHeight()
                .HAlign(HAlign_Right)
                [
                    SNew(SHorizontalBox)

                    // Resource name
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
                            .Text(Resource->Name)
                        ]
                    ]

                    // Limits
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
                            .Text(UnloadLimits)
                        ]
                    ]

                    // Limit decrease
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SFlareButton)
                        .OnClicked(this, &SFlareTradeRouteMenu::OnDecreaseUnloadLimitClicked, Sector, Resource)
                        .Text(FText::FromString(TEXT("-")))
                        .Transparent(true)
                        .Width(1)
                    ]

                    // Limit increase
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SFlareButton)
                        .OnClicked(this, &SFlareTradeRouteMenu::OnIncreaseUnloadLimitClicked, Sector, Resource)
                        .Text(FText::FromString(TEXT("+")))
                        .Transparent(true)
                        .Width(1)
                    ]

                    // Clear resource load
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SFlareButton)
                        .OnClicked(this, &SFlareTradeRouteMenu::OnClearUnloadResourceClicked, Sector, Resource)
                        .Text(FText())
                        .HelpText(LOCTEXT("ClearUnloadResource", "Stop unloading this resource"))
                        .Icon(FFlareStyleSet::GetIcon("Stop"))
                        .Transparent(true)
                        .Width(1)
                    ]
                ];
            }
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
    return Item ? Item->GetSectorName() : LOCTEXT("SelectSector", "Select a sector");
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

TSharedRef<SWidget> SFlareTradeRouteMenu::OnGenerateResourceComboLine(UFlareResourceCatalogEntry* Item)
{
    const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

    return SNew(STextBlock)
    .Text(Item->Data.Name)
    .TextStyle(&Theme.TextFont);
}

void SFlareTradeRouteMenu::OnResourceComboLineSelectionChanged(UFlareResourceCatalogEntry* Item, ESelectInfo::Type SelectInfo)
{

}

FText SFlareTradeRouteMenu::OnGetCurrentResourceComboLine() const
{
    UFlareResourceCatalogEntry* Item = ResourceSelector->GetSelectedItem();
    return Item ? Item->Data.Name : LOCTEXT("SelectResource", "Select a resource");
}

EVisibility SFlareTradeRouteMenu::GetResourceSelectorVisibility() const
{
    EVisibility visibility = EVisibility::Collapsed;
    if(TargetTradeRoute)
    {
        if(TargetTradeRoute->GetSectors().Num() > 0)
        {
            visibility = EVisibility::Visible;
        }
    }
    return visibility;
}


void SFlareTradeRouteMenu::OnLoadResourceClicked(UFlareSimulatedSector* Sector)
{
    UFlareResourceCatalogEntry* Resource = ResourceSelector->GetSelectedItem();
    int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
    if(SectorIndex >= 0 && Resource)
    {
        TargetTradeRoute->SetSectorLoadOrder(SectorIndex, &Resource->Data, 0);
        GenerateSectorList();
    }
}

void SFlareTradeRouteMenu::OnDecreaseLoadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource)
{
    int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
    FFlareTradeRouteSectorSave* PreviousOrder = TargetTradeRoute->GetSectorOrders(Sector);

    if(SectorIndex >= 0 && Resource && PreviousOrder)
    {
        uint32 PreviousLimit = 0;
        for (int ResourceIndex = 0; ResourceIndex < PreviousOrder->ResourcesToLoad.Num() ; ResourceIndex++)
        {
            if(Resource->Identifier == PreviousOrder->ResourcesToLoad[ResourceIndex].ResourceIdentifier)
            {
                PreviousLimit = PreviousOrder->ResourcesToLoad[ResourceIndex].Quantity;
                break;
            }
        }

        if(PreviousLimit == 0)
        {
            return;
        }


        TargetTradeRoute->SetSectorLoadOrder(SectorIndex, Resource, PreviousLimit - 1);
        GenerateSectorList();
    }
}

void SFlareTradeRouteMenu::OnIncreaseLoadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource)
{
    int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
    FFlareTradeRouteSectorSave* PreviousOrder = TargetTradeRoute->GetSectorOrders(Sector);

    if(SectorIndex >= 0 && Resource && PreviousOrder)
    {
        uint32 PreviousLimit = 0;
        for (int ResourceIndex = 0; ResourceIndex < PreviousOrder->ResourcesToLoad.Num() ; ResourceIndex++)
        {
            if(Resource->Identifier == PreviousOrder->ResourcesToLoad[ResourceIndex].ResourceIdentifier)
            {
                PreviousLimit = PreviousOrder->ResourcesToLoad[ResourceIndex].Quantity;
                break;
            }
        }

        TargetTradeRoute->SetSectorLoadOrder(SectorIndex, Resource, PreviousLimit + 1);
        GenerateSectorList();
    }
}


void SFlareTradeRouteMenu::OnClearLoadResourceClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource)
{
    int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
    if(SectorIndex >= 0 && Resource)
    {
        TargetTradeRoute->ClearSectorLoadOrder(SectorIndex, Resource);
        GenerateSectorList();
    }
}


void SFlareTradeRouteMenu::OnUnloadResourceClicked(UFlareSimulatedSector* Sector)
{
    UFlareResourceCatalogEntry* Resource = ResourceSelector->GetSelectedItem();
    int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
    if(SectorIndex >= 0 && Resource)
    {
        TargetTradeRoute->SetSectorUnloadOrder(SectorIndex, &Resource->Data, 0);
        GenerateSectorList();
    }
}


void SFlareTradeRouteMenu::OnDecreaseUnloadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource)
{
    int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
    FFlareTradeRouteSectorSave* PreviousOrder = TargetTradeRoute->GetSectorOrders(Sector);

    if(SectorIndex >= 0 && Resource && PreviousOrder)
    {
        uint32 PreviousLimit = 0;
        for (int ResourceIndex = 0; ResourceIndex < PreviousOrder->ResourcesToUnload.Num() ; ResourceIndex++)
        {
            if(Resource->Identifier == PreviousOrder->ResourcesToUnload[ResourceIndex].ResourceIdentifier)
            {
                PreviousLimit = PreviousOrder->ResourcesToUnload[ResourceIndex].Quantity;
                break;
            }
        }

        if(PreviousLimit == 0)
        {
            return;
        }


        TargetTradeRoute->SetSectorUnloadOrder(SectorIndex, Resource, PreviousLimit - 1);
        GenerateSectorList();
    }
}

void SFlareTradeRouteMenu::OnIncreaseUnloadLimitClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource)
{
    int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
    FFlareTradeRouteSectorSave* PreviousOrder = TargetTradeRoute->GetSectorOrders(Sector);

    if(SectorIndex >= 0 && Resource && PreviousOrder)
    {
        uint32 PreviousLimit = 0;
        for (int ResourceIndex = 0; ResourceIndex < PreviousOrder->ResourcesToUnload.Num() ; ResourceIndex++)
        {
            if(Resource->Identifier == PreviousOrder->ResourcesToUnload[ResourceIndex].ResourceIdentifier)
            {
                PreviousLimit = PreviousOrder->ResourcesToUnload[ResourceIndex].Quantity;
                break;
            }
        }

        TargetTradeRoute->SetSectorUnloadOrder(SectorIndex, Resource, PreviousLimit + 1);
        GenerateSectorList();
    }
}


void SFlareTradeRouteMenu::OnClearUnloadResourceClicked(UFlareSimulatedSector* Sector, FFlareResourceDescription* Resource)
{
    int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
    if(SectorIndex >= 0 && Resource)
    {
        TargetTradeRoute->ClearSectorUnloadOrder(SectorIndex, Resource);
        GenerateSectorList();
    }
}


#undef LOCTEXT_NAMESPACE

