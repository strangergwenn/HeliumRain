
#include "FlareTradeRouteMenu.h"
#include "../../Flare.h"

#include "../../Data/FlareResourceCatalog.h"

#include "../../Game/FlareGame.h"
#include "../../Game/FlareGameTools.h"
#include "../../Game/FlareTradeRoute.h"
#include "../../Player/FlareMenuManager.h"
#include "../../Player/FlarePlayerController.h"

#include "../Components/FlareButton.h"
#include "../Components/FlareSectorButton.h"


#define LOCTEXT_NAMESPACE "FlareTradeRouteMenu"

#define MAX_WAIT_LIMIT 29


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareTradeRouteMenu::Construct(const FArguments& InArgs)
{
	// Data
	MenuManager = InArgs._MenuManager;
	AFlarePlayerController* PC = MenuManager->GetPC();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	TArray<UFlareResourceCatalogEntry*> ResourceList = PC->GetGame()->GetResourceCatalog()->Resources;
	MaxSectorsInRoute = 4;

	OperationList.Add(EFlareTradeRouteOperation::Load);
	OperationNameList.Add(MakeShareable(new FText(LOCTEXT("OpLoad", "Load"))));
	OperationList.Add(EFlareTradeRouteOperation::Buy);
	OperationNameList.Add(MakeShareable(new FText(LOCTEXT("OpBuy", "Buy"))));
	OperationList.Add(EFlareTradeRouteOperation::LoadOrBuy);
	OperationNameList.Add(MakeShareable(new FText(LOCTEXT("OpLoadBuy", "Load / Buy"))));
	OperationList.Add(EFlareTradeRouteOperation::Unload);
	OperationNameList.Add(MakeShareable(new FText(LOCTEXT("OpUnload", "Unload"))));
	OperationList.Add(EFlareTradeRouteOperation::Sell);
	OperationNameList.Add(MakeShareable(new FText(LOCTEXT("OpSell", "Sell"))));
	OperationList.Add(EFlareTradeRouteOperation::UnloadOrSell);
	OperationNameList.Add(MakeShareable(new FText(LOCTEXT("OpUnloadSell", "Unload / Sell"))));

	// Build structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, AFlareMenuManager::GetMainOverlayHeight(), 0, 0))
	[
		SNew(SVerticalBox)

		// UI container
		+ SVerticalBox::Slot()
		[
			SNew(SHorizontalBox)

			// Side panel
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			.Padding(Theme.ContentPadding)
			[
				SNew(SVerticalBox)
				
				// Title
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.TitlePadding)
				[
					SNew(STextBlock)
					.Text(this, &SFlareTradeRouteMenu::GetTradeRouteName)
					.TextStyle(&Theme.SubTitleFont)
				]

				// Status of the current fleet
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Left)
				[
					SNew(SRichTextBlock)
					.Text(this, &SFlareTradeRouteMenu::GetFleetInfo)
					.TextStyle(&Theme.TextFont)
					.DecoratorStyleSet(&FFlareStyleSet::Get())
					.WrapTextAt(Theme.ContentWidth / 2)
				]
				
				// Sector selection
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					// Name field
					+ SHorizontalBox::Slot()
					.Padding(Theme.SmallContentPadding)
					.VAlign(VAlign_Center)
					[
						SAssignNew(EditRouteName, SEditableText)
						.AllowContextMenu(false)
						.Style(&Theme.TextInputStyle)
					]

					// Confirm
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					.HAlign(HAlign_Right)
					[
						SNew(SFlareButton)
						.Width(3)
						.Text(LOCTEXT("Rename", "Rename"))
						.HelpText(LOCTEXT("ChangeNameInfo", "Rename this trade route"))
						.Icon(FFlareStyleSet::GetIcon("OK"))
						.OnClicked(this, &SFlareTradeRouteMenu::OnConfirmChangeRouteNameClicked)
						.IsDisabled(this, &SFlareTradeRouteMenu::IsRenameDisabled)
					]
				]
				
				// Fleet selection
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					// List
					+ SHorizontalBox::Slot()
					.Padding(Theme.SmallContentPadding)
					[
						SAssignNew(FleetSelector, SFlareDropList<UFlareFleet*>)
						.OptionsSource(&FleetList)
						.OnGenerateWidget(this, &SFlareTradeRouteMenu::OnGenerateFleetComboLine)
						.OnSelectionChanged(this, &SFlareTradeRouteMenu::OnFleetComboLineSelectionChanged)
						.Visibility(this, &SFlareTradeRouteMenu::GetAssignFleetVisibility)
						.HeaderWidth(7)
						.ItemWidth(7)
						[
							SNew(SBox)
							.Padding(Theme.ListContentPadding)
							[
								SNew(STextBlock)
								.Text(this, &SFlareTradeRouteMenu::OnGetCurrentFleetComboLine)
								.TextStyle(&Theme.TextFont)
							]
						]
					]

					// Name field
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Top)
					[
						SNew(SFlareButton)
						.Width(3)
						.Text(LOCTEXT("AssignFleet", "Assign"))
						.HelpText(LOCTEXT("AssignFleetInfo", "Assign this fleet to the trade route"))
						.OnClicked(this, &SFlareTradeRouteMenu::OnAssignFleetClicked)
						.Visibility(this, &SFlareTradeRouteMenu::GetAssignFleetVisibility)
					]
				]

				// Fleet list (assigned)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SAssignNew(TradeFleetList, SVerticalBox)
				]
				
				// Next trade operation
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					// Status of the next trade step
					+ SHorizontalBox::Slot()
					.Padding(Theme.SmallContentPadding)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(this, &SFlareTradeRouteMenu::GetNextStepInfo)
						.TextStyle(&Theme.TextFont)
						.ColorAndOpacity(Theme.InfoColor)
					]

					// Skip next operation
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					.HAlign(HAlign_Right)
					[
						SNew(SFlareButton)
						.Width(1.9)
						.Text(LOCTEXT("SkipOperation", "Skip"))
						.HelpText(LOCTEXT("SkipOperationInfo", "Skip the next operation and move on with the following one"))
						.OnClicked(this, &SFlareTradeRouteMenu::OnSkipOperationClicked)
					]

					// Pause
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					.HAlign(HAlign_Right)
					[
						SNew(SFlareButton)
						.Width(1)
						.Icon(this, &SFlareTradeRouteMenu::GetPauseIcon)
						.Text(FText())
						.HelpText(LOCTEXT("PauseInfo", "Pause the trade route"))
						.OnClicked(this, &SFlareTradeRouteMenu::OnPauseTradeRouteClicked)
					]
				]
					
				// Reset
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				.HAlign(HAlign_Left)
				[
					SNew(SFlareButton)
					.Width(4)
					.Icon(FFlareStyleSet::GetIcon("OK"))
					.Text(LOCTEXT("Reset", "Reset statistics"))
					.HelpText(LOCTEXT("ResetInfo", "Reinitialize statistics for this trade route"))
					.OnClicked(this, &SFlareTradeRouteMenu::OnResetStatistics)
				]

				// Operation edition box
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SVerticalBox)
					.Visibility(this, &SFlareTradeRouteMenu::GetOperationDetailsVisibility)

					// Title resource
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.TitlePadding)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("OperationEditTitle", "Edit selected operation"))
						.TextStyle(&Theme.SubTitleFont)
					]

					// Status of the selected trade step
					+ SVerticalBox::Slot()
					.Padding(Theme.SmallContentPadding)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(this, &SFlareTradeRouteMenu::GetSelectedStepInfo)
						.TextStyle(&Theme.TextFont)
						.ColorAndOpacity(Theme.ObjectiveColor)
					]
					
					// Resource action
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.SmallContentPadding)
					[
						SAssignNew(OperationSelector, SFlareDropList<TSharedPtr<FText>>)
						.OptionsSource(&OperationNameList)
						.OnGenerateWidget(this, &SFlareTradeRouteMenu::OnGenerateOperationComboLine)
						.OnSelectionChanged(this, &SFlareTradeRouteMenu::OnOperationComboLineSelectionChanged)
						.HeaderWidth(10)
						.ItemWidth(10)
						[
							SNew(SBox)
							.Padding(Theme.ListContentPadding)
							[
								SNew(STextBlock)
								.Text(this, &SFlareTradeRouteMenu::OnGetCurrentOperationComboLine)
								.TextStyle(&Theme.TextFont)
							]
						]
					]

					// Resource selection
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.SmallContentPadding)
					[
						SAssignNew(ResourceSelector, SFlareDropList<UFlareResourceCatalogEntry*>)
						.OptionsSource(&PC->GetGame()->GetResourceCatalog()->Resources)
						.OnGenerateWidget(this, &SFlareTradeRouteMenu::OnGenerateResourceComboLine)
						.OnSelectionChanged(this, &SFlareTradeRouteMenu::OnResourceComboLineSelectionChanged)
						.HeaderWidth(10)
						.ItemWidth(10)
						[
							SNew(SBox)
							.Padding(Theme.ListContentPadding)
							[
								SNew(STextBlock)
								.Text(this, &SFlareTradeRouteMenu::OnGetCurrentResourceComboLine)
								.TextStyle(&Theme.TextFont)
							]
						]
					]
					
					// Operation order
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Move up
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SFlareButton)
							.OnClicked(this, &SFlareTradeRouteMenu::OnOperationUpClicked)
							.Text(LOCTEXT("MoveUpOperation", "Move up"))
						]

						// Move down
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SFlareButton)
							.OnClicked(this, &SFlareTradeRouteMenu::OnOperationDownClicked)
							.Text(LOCTEXT("MoveDownOperation", "Move down"))
						]
					]
					
					// Resource setup
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						// Left field
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SVerticalBox)

							// Quantity limit
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.SmallContentPadding)
							[
								SAssignNew(QuantityLimitButton, SFlareButton)
								.Text(LOCTEXT("QuantityLimit", "Limit max quantity"))
								.HelpText(LOCTEXT("QuantityLimitInfo", "Skip to the next operation after a certain amount is is reached"))
								.Toggle(true)
								.OnClicked(this, &SFlareTradeRouteMenu::OnQuantityLimitToggle)
							]

							// Quantity
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(SHorizontalBox)

								// Quantity limit slider
								+ SHorizontalBox::Slot()
								.HAlign(HAlign_Fill)
								.Padding(Theme.SmallContentPadding)
								[
									SAssignNew(QuantityLimitSlider, SSlider)
									.Style(&Theme.SliderStyle)
									.Value(0)
									.OnValueChanged(this, &SFlareTradeRouteMenu::OnQuantityLimitChanged)
									.Visibility(this, &SFlareTradeRouteMenu::GetQuantityLimitVisibility)
								]

								// Text box
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.HAlign(HAlign_Right)
								.Padding(Theme.ContentPadding)
								[
									SAssignNew(QuantityLimitText, SEditableText)
									.Style(&Theme.TextInputStyle)
									.OnTextChanged(this, &SFlareTradeRouteMenu::OnQuantityLimitEntered)
									.Visibility(this, &SFlareTradeRouteMenu::GetQuantityLimitVisibility)
								]
							]
						]
					
						// Right field
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SVerticalBox)

							// Wait limit
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.SmallContentPadding)
							[
								SAssignNew(WaitLimitButton, SFlareButton)
								.Text(LOCTEXT("WaitLimit", "Limit max wait"))
								.HelpText(LOCTEXT("WaitLimitInfo", "Skip to the next operation after some time has passed"))
								.Toggle(true)
								.OnClicked(this, &SFlareTradeRouteMenu::OnWaitLimitToggle)
							]

							// Wait limit slider
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(Theme.SmallContentPadding)
							[
								SAssignNew(WaitLimitSlider, SSlider)
								.Style(&Theme.SliderStyle)
								.Value(0)
								.OnValueChanged(this, &SFlareTradeRouteMenu::OnWaitLimitChanged)
								.Visibility(this, &SFlareTradeRouteMenu::GetWaitLimitVisibility)
							]
						]
					]

					// Done editing
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Left)
					[
						SNew(SFlareButton)
						.Width(4)
						.Icon(FFlareStyleSet::GetIcon("OK"))
						.Text(LOCTEXT("DoneEditing", "Done"))
						.HelpText(LOCTEXT("DoneEditingInfo", "Finish editing this operation"))
						.OnClicked(this, &SFlareTradeRouteMenu::OnDoneClicked)
					]
				]
			]
			
			// Content block
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Fill)
			.Padding(Theme.ContentPadding)
			[
				SNew(SScrollBox)
				.Style(&Theme.ScrollBoxStyle)
				.ScrollBarStyle(&Theme.ScrollBarStyle)

				+ SScrollBox::Slot()
				.HAlign(HAlign_Fill)
				[
					SNew(SVerticalBox)
				
					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.TitlePadding)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("TradeRouteSectorsTitle", "Add a sector"))
						.TextStyle(&Theme.SubTitleFont)
					]

					// Sector selection
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Fill)
					[
						SNew(SBox)
						.WidthOverride(Theme.ContentWidth)
						[
							SNew(SHorizontalBox)

							// List
							+ SHorizontalBox::Slot()
							.Padding(Theme.SmallContentPadding)
							.AutoWidth()
							[
								SAssignNew(SectorSelector, SFlareDropList<UFlareSimulatedSector*>)
								.OptionsSource(&SectorList)
								.OnGenerateWidget(this, &SFlareTradeRouteMenu::OnGenerateSectorComboLine)
								.OnSelectionChanged(this, &SFlareTradeRouteMenu::OnSectorComboLineSelectionChanged)
								.HeaderWidth(15)
								.HeaderHeight(2.5)
								.ItemWidth(15)
								.ItemHeight(2.5)
								.MaximumHeight(500)
								[
									SNew(SBox)
									.Padding(Theme.ListContentPadding)
									[
										SNew(STextBlock)
										.Text(this, &SFlareTradeRouteMenu::OnGetCurrentSectorComboLine)
										.TextStyle(&Theme.TextFont)
									]
								]
							]

							// Button
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(Theme.SmallContentPadding)
							.VAlign(VAlign_Top)
							[
								SNew(SFlareButton)
								.Width(3)
								.Text(this, &SFlareTradeRouteMenu::GetAddSectorText)
								.HelpText(LOCTEXT("AddSectorInfo", "Add this sector to the trade route"))
								.OnClicked(this, &SFlareTradeRouteMenu::OnAddSectorClicked)
								.IsDisabled(this, &SFlareTradeRouteMenu::IsAddSectorDisabled)
							]
						]
					]
				
					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.TitlePadding)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("TradeRouteMapTitle", "Flight plan"))
						.TextStyle(&Theme.SubTitleFont)
					]

					// Map
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Center)
					[
						SAssignNew(TradeSectorList, SHorizontalBox)
					]
				]
			]
		]
	];

	// Setup lists
	OperationSelector->RefreshOptions();
	ResourceSelector->RefreshOptions();
	if (ResourceList.Num() > 0)
	{
		ResourceSelector->SetSelectedItem(ResourceList[0]);
	}
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareTradeRouteMenu::Setup()
{
	SetEnabled(false);
	SetVisibility(EVisibility::Collapsed);
}

void SFlareTradeRouteMenu::Enter(UFlareTradeRoute* TradeRoute)
{
	FLOG("SFlareTradeMenu::Enter");

	SetEnabled(true);
	SetVisibility(EVisibility::Visible);

	TargetTradeRoute = TradeRoute;
	SelectedOperation = NULL;
	EditRouteName->SetText(GetTradeRouteName());
	GenerateSectorList();
	GenerateFleetList();
}

void SFlareTradeRouteMenu::Exit()
{
	SetEnabled(false);
	TargetTradeRoute = NULL;
	SelectedOperation = NULL;
	TradeSectorList->ClearChildren();
	TradeFleetList->ClearChildren();

	SetVisibility(EVisibility::Collapsed);
}

void SFlareTradeRouteMenu::GenerateSectorList()
{
	SectorList.Empty();
	TradeSectorList->ClearChildren();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	if (TargetTradeRoute)
	{
		// Fetch all known sectors
		TArray<UFlareSimulatedSector*>& VisitedSectors = MenuManager->GetGame()->GetPC()->GetCompany()->GetVisitedSectors();
		for (int SectorIndex = 0; SectorIndex < VisitedSectors.Num(); SectorIndex++)
		{
			if (!TargetTradeRoute->IsVisiting(VisitedSectors[SectorIndex]))
			{
				SectorList.Add(VisitedSectors[SectorIndex]);
			}
		}
		SectorSelector->RefreshOptions();

		// Iterate on the trade route
		TArray<FFlareTradeRouteSectorSave>& Sectors = TargetTradeRoute->GetSectors();
		for (int SectorIndex = 0; SectorIndex < Sectors.Num(); SectorIndex++)
		{
			TSharedPtr<SVerticalBox> SectorOperationList;
			FFlareTradeRouteSectorSave* SectorOrders = &Sectors[SectorIndex];
			UFlareSimulatedSector* Sector = MenuManager->GetGame()->GetGameWorld()->FindSector(SectorOrders->SectorIdentifier);

			TradeSectorList->AddSlot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			[
				SNew(SVerticalBox)

				// Sector info
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					// Arrow 1
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Top)
					.AutoWidth()
					[
						SNew(SImage)
						.Image(FFlareStyleSet::GetImage(SectorIndex > 0 ? "TradeRouteArrow2" : "TradeRouteArrow_Start"))
					]

					// Remove sector
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Top)
					.AutoWidth()
					[
						SNew(SFlareButton)
						.Transparent(true)
						.Text(FText())
						.HelpText(LOCTEXT("RemoveSectorHelp", "Remove this sector from the trade route"))
						.Icon(FFlareStyleSet::GetIcon("Stop"))
						.OnClicked(this, &SFlareTradeRouteMenu::OnRemoveSectorClicked, Sector)
						.Width(1)
					]

					// Sector info
					+ SHorizontalBox::Slot()
					[
						SNew(SFlareSectorButton)
						.Sector(Sector)
						.PlayerCompany(MenuManager->GetGame()->GetPC()->GetCompany())
					]

					// Arrow 2
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Top)
					.AutoWidth()
					[
						SNew(SImage)
						.Image(FFlareStyleSet::GetImage(SectorIndex < Sectors.Num() - 1 ? "TradeRouteArrow" : "TradeRouteArrow_End"))
					]
				]

				// Operation list
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(Theme.ContentPadding)
				[
					SNew(SVerticalBox)

					// Operation list
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Left)
					[
						SAssignNew(SectorOperationList, SVerticalBox)
					]

					// Operation list
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("SectorAddOperationTitle", "Add operation"))
						.TextStyle(&Theme.NameFont)
					]

					// Add operation button
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Left)
					[
						SNew(SFlareButton)
						.Width(5)
						.OnClicked(this, &SFlareTradeRouteMenu::OnAddOperationClicked, Sector)
						.Text(LOCTEXT("SectorAddOperation", "Add"))
						.Icon(FFlareStyleSet::GetIcon("New"))
						.HelpText(LOCTEXT("UnloadHelp", "Add an operation for this sector"))
					]

					// Move left / right
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(Theme.ContentPadding)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("SectorMoveTitle", "Move sector in route"))
						.TextStyle(&Theme.NameFont)
					]

					// Add operation button
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Left)
					[
						SNew(SHorizontalBox)

						// Left
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.HAlign(HAlign_Left)
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SFlareButton)
							.Width(2.5)
							.OnClicked(this, &SFlareTradeRouteMenu::OnMoveLeft, Sector)
							.IsDisabled(this, &SFlareTradeRouteMenu::IsMoveLeftDisabled, Sector)
							.Text(LOCTEXT("SectorMoveLeft", "Move"))
							.Icon(FFlareStyleSet::GetIcon("MoveLeft"))
							.HelpText(LOCTEXT("SectorMoveLeftHelp", "Move this sector before the previous one in the trade route"))
						]

						// Right
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.HAlign(HAlign_Right)
						.Padding(Theme.SmallContentPadding)
						[
							SNew(SFlareButton)
							.Width(2.4)
							.OnClicked(this, &SFlareTradeRouteMenu::OnMoveRight, Sector)
							.IsDisabled(this, &SFlareTradeRouteMenu::IsMoveRightDisabled, Sector)
							.Text(LOCTEXT("SectorMoveRight", "Move"))
							.Icon(FFlareStyleSet::GetIcon("MoveRight"))
							.HelpText(LOCTEXT("SectorMoveRightHelp", "Move this sector after the next one in the trade route"))
						]
					]
				]
			];

			// Fill operation list
			for (int OperationIndex = 0; OperationIndex < SectorOrders->Operations.Num(); OperationIndex++)
			{
				FFlareTradeRouteSectorOperationSave* Operation = &SectorOrders->Operations[OperationIndex];
				FText OperationResume = FText::FromString(FString::FromInt(OperationIndex + 1) + " - " + GetOperationInfo(Operation).ToString()); // FString needed here;

				// TODO current operation progress
				// Add operation limits
				SectorOperationList->AddSlot()
				.AutoHeight()
				.Padding(Theme.ContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.NameFont)
					.Text(OperationResume)
					.ColorAndOpacity(this, &SFlareTradeRouteMenu::GetOperationHighlight, Operation)
				];

				SectorOperationList->AddSlot()
				.AutoHeight()
				.Padding(Theme.SmallContentPadding)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(this, &SFlareTradeRouteMenu::GetOperationStatusText, Operation, SectorOrders->SectorIdentifier)
				];

				// Buttons
				SectorOperationList->AddSlot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					// Edit
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(SFlareButton)
						.Width(2.4)
						.Text(LOCTEXT("EditOperation", "Edit"))
						.HelpText(LOCTEXT("EditOperationInfo", "Edit this trade operation"))
						.IsDisabled(this, &SFlareTradeRouteMenu::IsEditOperationDisabled, Operation)
						.OnClicked(this, &SFlareTradeRouteMenu::OnEditOperationClicked, Operation)
					]

					// Delete
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Theme.SmallContentPadding)
					[
						SNew(SFlareButton)
						.Width(2.4)
						.Text(LOCTEXT("DeleteOperation", "Delete"))
						.HelpText(LOCTEXT("DeletOperationInfo", "Delete this trade operation"))
						.OnClicked(this, &SFlareTradeRouteMenu::OnDeleteOperationClicked, Operation)
					]
				];
			}
		}
	}

	if (SectorList.Num() > 0)
	{
		SectorSelector->SetSelectedItem(SectorList[0]);
	}

	SlatePrepass(FSlateApplicationBase::Get().GetApplicationScale());
}

void SFlareTradeRouteMenu::GenerateFleetList()
{
	FleetList.Empty();
	TradeFleetList->ClearChildren();
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	if (TargetTradeRoute)
	{
		// Available fleets (to combo box)
		TArray<UFlareFleet*>& Fleets = MenuManager->GetGame()->GetPC()->GetCompany()->GetCompanyFleets();
		for (int FleetIndex = 0; FleetIndex < Fleets.Num(); FleetIndex++)
		{
			if (!Fleets[FleetIndex]->GetCurrentTradeRoute() && Fleets[FleetIndex] != MenuManager->GetPC()->GetPlayerFleet())
			{
				FleetList.Add(Fleets[FleetIndex]);
			}
		}
		FleetSelector->RefreshOptions();

		// Assigned fleets (text display)
		if (TargetTradeRoute->GetFleet())
		{
			UFlareFleet* Fleet = TargetTradeRoute->GetFleet();

			TradeFleetList->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			[
				SNew(SHorizontalBox)

				// Fleet info
				+ SHorizontalBox::Slot()
				.Padding(Theme.SmallContentPadding)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.TextStyle(&Theme.TextFont)
					.Text(FText::Format(LOCTEXT("FleetNameFormat", "Assigned fleet : {0}"), Fleet->GetFleetName()))
				]

				// Unassign fleet
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(Theme.SmallContentPadding)
				.HAlign(HAlign_Right)
				[
					SNew(SFlareButton)
					.Width(3)
					.OnClicked(this, &SFlareTradeRouteMenu::OnUnassignFleetClicked, Fleet)
					.Text(FText(LOCTEXT("Unassign", "Unassign")))
					.HelpText(FText(LOCTEXT("UnassignHelp", "Unassign this fleet and pick another one")))
					.Icon(FFlareStyleSet::GetIcon("Stop"))
				]
			];
		}

		// No fleets
		if (FleetList.Num() == 0 && TargetTradeRoute->GetFleet() == NULL)
		{
			TradeFleetList->AddSlot()
			.AutoHeight()
			.Padding(Theme.SmallContentPadding)
			[
				SNew(STextBlock)
				.TextStyle(&Theme.TextFont)
				.Text(LOCTEXT("NoFleet", "No fleet assigned or available, route disabled"))
			];
		}
	}

	// Pre-select the first fleet
	if (FleetList.Num() > 0)
	{
		FleetSelector->SetSelectedItem(FleetList[0]);
	}

	SlatePrepass(FSlateApplicationBase::Get().GetApplicationScale());
}


/*----------------------------------------------------
	Content callbacks
----------------------------------------------------*/

FText SFlareTradeRouteMenu::GetTradeRouteName() const
{
	FText Result;

	if (TargetTradeRoute)
	{
		Result = TargetTradeRoute->GetTradeRouteName();
	}

	return Result;
}

FText SFlareTradeRouteMenu::GetFleetInfo() const
{
	FCHECK(TargetTradeRoute);
	if (TargetTradeRoute && TargetTradeRoute->GetFleet())
	{
		// Fleet info
		FText FleetInfo = FText::Format(LOCTEXT("FleetCargoInfoFormat", "\u2022 Current fleet status : {0} \n\u2022 Total fleet capacity : {1} ({2} free)"),
			TargetTradeRoute->GetFleet()->GetStatusInfo(),
			FText::AsNumber(TargetTradeRoute->GetFleet()->GetFleetCapacity()),
			FText::AsNumber(TargetTradeRoute->GetFleet()->GetFleetFreeCargoSpace()));

		// Get statistics
		const FFlareTradeRouteSave* TradeRouteData = TargetTradeRoute->Save();
		FCHECK(TradeRouteData);
		int32 TotalOperations = TradeRouteData->StatsOperationSuccessCount + TradeRouteData->StatsOperationFailCount;
		int32 SuccessPercentage = (TotalOperations > 0) ? (FMath::RoundToInt(100.0f * TradeRouteData->StatsOperationSuccessCount / float(TotalOperations))) : 0;
		int32 CreditsGain = (TradeRouteData->StatsDays > 0) ? (FMath::RoundToInt(0.01f * float(TradeRouteData->StatsMoneySell - TradeRouteData->StatsMoneyBuy) / float(TradeRouteData->StatsDays))) : 0;

		// Format statistics
		FText ResultInfo;
		if (CreditsGain > 0)
		{
			ResultInfo = FText::Format(LOCTEXT("TradeRouteDetailsGain", "\u2022 Average gain of <TradeText>{0} credits per day</>, {1}% of trade steps successful"),
				FText::AsNumber(CreditsGain),
				FText::AsNumber(SuccessPercentage));
		}
		else
		{
			ResultInfo = FText::Format(LOCTEXT("TradeRouteDetailsLoss", "\u2022 Average gain of <WarningText>{0} credits per day</>, {1}% of trade steps successful"),
				FText::AsNumber(CreditsGain),
				FText::AsNumber(SuccessPercentage));
		}

		// Format statistics
		FText LoadInfo = FText::Format(LOCTEXT("TradeRouteLoadInfo", "\u2022 {0} resources loaded, {1} resources unloaded"),
				FText::AsNumber(TradeRouteData->StatsLoadResources),
				FText::AsNumber(TradeRouteData->StatsUnloadResources));

		return FText::FromString(FleetInfo.ToString() + "\n" + ResultInfo.ToString() + "\n" + LoadInfo.ToString());
	}
	else
	{
		return LOCTEXT("NoFleetSelected", "\u2022 No assigned fleet !");
	}
}

FText SFlareTradeRouteMenu::GetSelectedStepInfo() const
{
	if (SelectedOperation)
	{
		return FText::Format(LOCTEXT("SelectedSectorFormat", "Operation : {0}"),
			GetOperationInfo(SelectedOperation));
	}
	
	return FText();
}

FText SFlareTradeRouteMenu::GetNextStepInfo() const
{
	if (TargetTradeRoute)
	{
		UFlareSimulatedSector* NextSector = TargetTradeRoute->GetTargetSector();
		FFlareTradeRouteSave* TradeRouteData = TargetTradeRoute->Save();
		FCHECK(TradeRouteData);

		if (TargetTradeRoute->IsPaused())
		{
			return LOCTEXT("PausedStep", "Paused");
		}
		else if (NextSector)
		{
			return FText::Format(LOCTEXT("NextSectorFormat", "Next operation : {0}"),
				GetOperationInfo(TargetTradeRoute->GetActiveOperation()));
		}
	}

	return LOCTEXT("NoNextStep", "No trade step defined");
}

const FSlateBrush* SFlareTradeRouteMenu::GetPauseIcon() const
{
	if (TargetTradeRoute)
	{
		if (TargetTradeRoute->IsPaused())
		{
			return FFlareStyleSet::GetIcon("Load");
		}
		else
		{
			return FFlareStyleSet::GetIcon("Pause");
		}
	}

	return NULL;
}

FText SFlareTradeRouteMenu::GetOperationInfo(FFlareTradeRouteSectorOperationSave* Operation) const
{
	if (TargetTradeRoute && Operation)
	{
		FFlareResourceDescription* Resource = MenuManager->GetGame()->GetResourceCatalog()->Get(Operation->ResourceIdentifier);
		int32 OperationNameIndex = OperationList.Find(Operation->Type);

		if (Resource && OperationNameIndex >= 0 && OperationNameIndex < OperationNameList.Num())
		{
			return FText::Format(LOCTEXT("OperationInfoFormat", "{0} {1}"),
				*OperationNameList[OperationNameIndex],
				Resource->Acronym);
		}
	}

	return LOCTEXT("NoNextOperation", "No operation");
}

TSharedRef<SWidget> SFlareTradeRouteMenu::OnGenerateSectorComboLine(UFlareSimulatedSector* TargetSector)
{
	// Common resources
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	UFlareSimulatedPlanetarium* Planetarium = MenuManager->GetGame()->GetGameWorld()->GetPlanerarium();
	FFlareCelestialBody* CelestialBody = Planetarium->FindCelestialBody(TargetSector->GetOrbitParameters()->CelestialBodyIdentifier);
	UFlareResourceCatalog* ResourceCatalog = MenuManager->GetGame()->GetResourceCatalog();

	// Resource lists
	TArray<FFlareResourceDescription*> CurrentlyBoughtResources;
	TArray<FFlareResourceDescription*> CurrentlySoldResources;
	TArray<FFlareResourceDescription*> SellableResources;
	TArray<FFlareResourceDescription*> BuyableResources;
	TArray<FFlareResourceDescription*> BestSellableResources;
	TArray<FFlareResourceDescription*> BestBuyableResources;

	// Identify all resources used
	for (FFlareTradeRouteSectorSave& Sector : TargetTradeRoute->GetSectors())
	{
		for (FFlareTradeRouteSectorOperationSave& Operation : Sector.Operations)
		{
			FFlareResourceDescription* Resource = ResourceCatalog->Get(Operation.ResourceIdentifier);
			switch (Operation.Type)
			{
				case EFlareTradeRouteOperation::Load:
				case EFlareTradeRouteOperation::Buy:
				case EFlareTradeRouteOperation::LoadOrBuy:
					CurrentlyBoughtResources.AddUnique(Resource);
					break;

				case EFlareTradeRouteOperation::Unload:
				case EFlareTradeRouteOperation::Sell:
				case EFlareTradeRouteOperation::UnloadOrSell:
					CurrentlySoldResources.AddUnique(Resource);
					break;
			}
		}
	}

	// Find buyers and sellers, then add all resources
	for (FFlareResourceDescription* Resource : CurrentlyBoughtResources)
	{
		if (TargetSector->WantBuy(Resource, MenuManager->GetPC()->GetCompany()))
		{
			SellableResources.AddUnique(Resource);
			BestSellableResources.AddUnique(Resource);
		}
	}
	for (FFlareResourceDescription* Resource : CurrentlySoldResources)
	{
		if (TargetSector->WantSell(Resource, MenuManager->GetPC()->GetCompany()))
		{
			BuyableResources.AddUnique(Resource);
			BestBuyableResources.AddUnique(Resource);
		}
	}
	for (UFlareResourceCatalogEntry* ResourceEntry : ResourceCatalog->GetResourceList())
	{
		FFlareResourceDescription* Resource = &ResourceEntry->Data;
		if (TargetSector->WantBuy(Resource, MenuManager->GetPC()->GetCompany()))
		{
			SellableResources.AddUnique(Resource);
		}
		if (TargetSector->WantSell(Resource, MenuManager->GetPC()->GetCompany()))
		{
			BuyableResources.AddUnique(Resource);
		}
	}

	// Build layout
	TSharedPtr<SHorizontalBox> SuggestedPurchasesBox;
	TSharedPtr<SHorizontalBox> SuggestedSalesBox;
	TSharedPtr<SWidget> Layout = SNew(SBox)
	.Padding(Theme.ListContentPadding)
	[
		SNew(SHorizontalBox)

		// Sector info
		+ SHorizontalBox::Slot()
		[
			SNew(STextBlock)
			.Text(FText::Format(LOCTEXT("SectorOrbitingFormat", "{0}\n\u2022 Orbiting {1}"), TargetSector->GetSectorName(), CelestialBody->Name))
			.TextStyle(&Theme.TextFont)
		]

		// Sold resources
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.Padding(Theme.SmallContentPadding)
		[
			SNew(SBox)
			.WidthOverride(3 * Theme.ResourceWidth)
			.Padding(FMargin(0))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.SmallContentPadding)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("SectorLineSellers", "Suggested purchases"))
					.TextStyle(&Theme.TextFont)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(SuggestedPurchasesBox, SHorizontalBox)
				]
			]
		]
		
		// Bought resources
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.Padding(Theme.SmallContentPadding)
		[
			SNew(SBox)
			.WidthOverride(3 * Theme.ResourceWidth)
			.Padding(FMargin(0))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(Theme.SmallContentPadding)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("SectorLineBuyers", "Suggested sales"))
					.TextStyle(&Theme.TextFont)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(SuggestedSalesBox, SHorizontalBox)
				]
			]
		]
	];

	// Add deals
	AddResourceDeals(SuggestedPurchasesBox, BuyableResources, BestBuyableResources);
	AddResourceDeals(SuggestedSalesBox, SellableResources, BestSellableResources);

	return Layout.ToSharedRef();
}


void SFlareTradeRouteMenu::AddResourceDeals(TSharedPtr<SHorizontalBox> ResourcesBox, TArray<FFlareResourceDescription*> Resources, TArray<FFlareResourceDescription*> BestResources)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	
	for (int i = 0; i < 3; i++)
	{
		// Resource
		if (i < Resources.Num())
		{
			FFlareResourceDescription* Resource = Resources[i];
			const FSlateBrush* InfoIcon = FFlareStyleSet::GetIcon("EmptySmall");
			if (BestResources.Find(Resource) != INDEX_NONE)
			{
				InfoIcon = FFlareStyleSet::GetIcon("Owned");
			}

			ResourcesBox->AddSlot()
			.AutoWidth()
			[
				SNew(SBorder)
				.Padding(FMargin(0))
				.BorderImage(&Resource->Icon)
				[
					SNew(SBox)
					.WidthOverride(Theme.ResourceWidth)
					.HeightOverride(Theme.ResourceHeight)
					.Padding(FMargin(0))
					[
						SNew(SVerticalBox)
			
						// Resource name
						+ SVerticalBox::Slot()
						.Padding(Theme.SmallContentPadding)
						[
							SNew(STextBlock)
							.TextStyle(&Theme.TextFont)
							.Text(Resource->Acronym)
						]

						// Deal info
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(Theme.SmallContentPadding)
						.VAlign(VAlign_Bottom)
						.HAlign(HAlign_Center)
						[
							SNew(SImage)
							.Image(InfoIcon)
						]
					]
				]
			];
		}

		// Empty
		else
		{
			ResourcesBox->AddSlot()
			.AutoWidth()
			[
				SNew(SBorder)
				.Padding(FMargin(0))
				.BorderImage(&Theme.ResourceBackground)
				[
					SNew(SBox)
					.WidthOverride(Theme.ResourceWidth)
					.HeightOverride(Theme.ResourceHeight)
					.Padding(FMargin(0))
				]
			];
		}

	}
}

void SFlareTradeRouteMenu::OnSectorComboLineSelectionChanged(UFlareSimulatedSector* Item, ESelectInfo::Type SelectInfo)
{
}

FText SFlareTradeRouteMenu::OnGetCurrentSectorComboLine() const
{
	UFlareSimulatedSector* Item = SectorSelector->GetSelectedItem();
	return Item ? Item->GetSectorName() : LOCTEXT("SelectSector", "Select a sector");
}

bool SFlareTradeRouteMenu::IsAddSectorDisabled() const
{
	if (TargetTradeRoute)
	{
		if (TargetTradeRoute->GetSectors().Num() >= MaxSectorsInRoute)
		{
			return true;
		}
	}

	if (SectorList.Num() == 0)
	{
		return true;
	}

	return false;
}

bool SFlareTradeRouteMenu::IsRenameDisabled() const
{
	if (TargetTradeRoute && TargetTradeRoute->GetTradeRouteName().ToString() != EditRouteName->GetText().ToString())
	{
		return false;
	}
	else
	{
		return true;
	}
}

TSharedRef<SWidget> SFlareTradeRouteMenu::OnGenerateResourceComboLine(UFlareResourceCatalogEntry* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(SBox)
	.Padding(Theme.ListContentPadding)
	[
		SNew(STextBlock)
		.Text(Item->Data.Name)
		.TextStyle(&Theme.TextFont)
	];
}

FText SFlareTradeRouteMenu::OnGetCurrentResourceComboLine() const
{
	UFlareResourceCatalogEntry* Item = ResourceSelector->GetSelectedItem();
	if (Item)
	{
		return FText::Format(LOCTEXT("ComboResourceLineFormat", "{0} ({1})"), Item->Data.Name, Item->Data.Acronym);
	}
	else
	{
		return LOCTEXT("SelectResource", "Select a resource");
	}
}

TSharedRef<SWidget> SFlareTradeRouteMenu::OnGenerateOperationComboLine(TSharedPtr<FText> Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(SBox)
	.Padding(Theme.ListContentPadding)
	[
		SNew(STextBlock)
		.Text(*Item) // FString needed here
		.TextStyle(&Theme.TextFont)
	];
}

FText SFlareTradeRouteMenu::OnGetCurrentOperationComboLine() const
{
	TSharedPtr<FText> Item = OperationSelector->GetSelectedItem();
	return Item.IsValid() ? *Item : *OperationNameList[0];
}

EVisibility SFlareTradeRouteMenu::GetOperationDetailsVisibility() const
{
	EVisibility visibility = EVisibility::Hidden;

	if (TargetTradeRoute)
	{
		if (SelectedOperation != NULL)
		{
			visibility = EVisibility::Visible;
		}
	}

	return visibility;
}

FText SFlareTradeRouteMenu::GetAddSectorText() const
{
	if (TargetTradeRoute)
	{
		return FText::Format(LOCTEXT("AddSectorFormat", "Add ({0} / {1})"),
			FText::AsNumber(TargetTradeRoute->GetSectors().Num()),
			FText::AsNumber(MaxSectorsInRoute));
	}

	return FText();
}

FSlateColor SFlareTradeRouteMenu::GetOperationHighlight(FFlareTradeRouteSectorOperationSave* Operation) const
{
	FLinearColor Result = FLinearColor::White;

	if (TargetTradeRoute)
	{
		const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

		if (SelectedOperation == Operation)
		{
			return Theme.ObjectiveColor;
		}
		else if (TargetTradeRoute->GetActiveOperation() == Operation)
		{
			return Theme.InfoColor;
		}
	}

	return Result;
}

FText SFlareTradeRouteMenu::GetOperationStatusText(FFlareTradeRouteSectorOperationSave* Operation, FName SectorName) const
{
	if (TargetTradeRoute && Operation)
	{
		FText WaitText;
		FText QuantityText;
		FFlareResourceDescription* Resource = MenuManager->GetGame()->GetResourceCatalog()->Get(Operation->ResourceIdentifier);
		
		// Time limit
		if (Operation->MaxWait == -1)
		{
			WaitText = LOCTEXT("Forever", "forever");
		}
		else
		{
			WaitText = FText::Format(LOCTEXT("OperationStatusActiveWaitFormat", "{0} days"),
				FText::AsNumber(Operation->MaxWait));
		}

		// Quantity limit
		if (Operation->MaxQuantity > -1)
		{
			QuantityText = FText::Format(LOCTEXT("OperationStatusActiveQuantityFormat", "{0} {1}"),
				FText::AsNumber(Operation->MaxQuantity),
				Resource->Acronym);
		}
		else
		{
			if (Operation->Type == EFlareTradeRouteOperation::Load
			 || Operation->Type == EFlareTradeRouteOperation::LoadOrBuy
			 || Operation->Type == EFlareTradeRouteOperation::Buy)
			{
				QuantityText = LOCTEXT("FullCargo", "until full");
			}
			else
			{
				QuantityText = LOCTEXT("EmptyCargo", "until empty");
			}
		}

		FText PricePart;

		if (Operation->Type == EFlareTradeRouteOperation::LoadOrBuy
		 || Operation->Type == EFlareTradeRouteOperation::UnloadOrSell
		|| Operation->Type == EFlareTradeRouteOperation::Buy
		|| Operation->Type == EFlareTradeRouteOperation::Sell)
		{
			UFlareSimulatedSector* Sector = MenuManager->GetGame()->GetGameWorld()->FindSector(SectorName);

			if (Resource && Sector)
			{


				int64 TransactionResourcePrice = Sector->GetResourcePrice(Resource,
														  (Operation->Type == EFlareTradeRouteOperation::Sell || Operation->Type == EFlareTradeRouteOperation::UnloadOrSell ? EFlareResourcePriceContext::FactoryInput: EFlareResourcePriceContext::FactoryOutput));
				int64 BaseResourcePrice = Sector->GetResourcePrice(Resource, EFlareResourcePriceContext::Default);
				int64 Fee = TransactionResourcePrice - BaseResourcePrice;

				PricePart = FText::Format(LOCTEXT("TradeUnitPriceFormat", "\n{0} credits/unit ({1} {2} {3} fee)"),
															UFlareGameTools::DisplayMoney(TransactionResourcePrice),
															UFlareGameTools::DisplayMoney(BaseResourcePrice),
															(Fee < 0 ? LOCTEXT("Minus", "-"): LOCTEXT("Plus", "+")),
															UFlareGameTools::DisplayMoney(FMath::Abs(Fee)));
			}
		}

		FText CommonPart = FText::Format(LOCTEXT("OtherOperationStatusFormat", "Wait {0} or {1}{2}"),
			WaitText,
			QuantityText,
			PricePart);

		// This is the active operation, add current progress
		if (TargetTradeRoute->GetActiveOperation() == Operation)
		{
			FText CurrentWaitText = FText::Format(LOCTEXT("OperationCurrentTimeFormat", "{0} days"),
				FText::AsNumber(TargetTradeRoute->GetData()->CurrentOperationDuration));

			FText CurrentQuantityText = FText::Format(LOCTEXT("OperationCurrentCargoFormat", "{0} {1}"),
				FText::AsNumber(TargetTradeRoute->GetData()->CurrentOperationProgress),
				Resource->Acronym);

			return FText::Format(LOCTEXT("CurrentOperationStatusFormat", "{0}\n({1}, {2} traded)"),
				CommonPart,
				CurrentWaitText,
				CurrentQuantityText);
		}

		// This is another operation; keep it simple
		else
		{
			return CommonPart;
		}
	}

	return FText();
}

FText SFlareTradeRouteMenu::GetLoadText() const
{
	FText Result;
	UFlareResourceCatalogEntry* Resource = ResourceSelector->GetSelectedItem();

	if (TargetTradeRoute && Resource)
	{
		Result = FText::Format(LOCTEXT("LoadFormat", "Load {0}"), Resource->Data.Acronym);
	}
	else
	{
		Result = LOCTEXT("LoadImpossible", "Load");
	}

	return Result;
}

FText SFlareTradeRouteMenu::GetUnloadText() const
{
	FText Result;
	UFlareResourceCatalogEntry* Resource = ResourceSelector->GetSelectedItem();

	if (TargetTradeRoute && Resource)
	{
		Result = FText::Format(LOCTEXT("UnloadFormat", "Unload {0}"), Resource->Data.Acronym);
	}
	else
	{
		Result = LOCTEXT("UnloadImpossible", "Select");
	}

	return Result;
}

TSharedRef<SWidget> SFlareTradeRouteMenu::OnGenerateFleetComboLine(UFlareFleet* Item)
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	return SNew(SBox)
	.Padding(Theme.ListContentPadding)
	[
		SNew(STextBlock)
		.Text(Item->GetFleetName())
		.TextStyle(&Theme.TextFont)
	];
}

FText SFlareTradeRouteMenu::OnGetCurrentFleetComboLine() const
{
	UFlareFleet* Item = FleetSelector->GetSelectedItem();
	return Item ? Item->GetFleetName() : LOCTEXT("SelectFleet", "Select a fleet");
}

EVisibility SFlareTradeRouteMenu::GetAssignFleetVisibility() const
{
	// Only one fleet !
	return FleetList.Num() > 0 && TargetTradeRoute && TargetTradeRoute->GetFleet() == NULL ? EVisibility::Visible : EVisibility::Collapsed;
}

bool SFlareTradeRouteMenu::IsEditOperationDisabled(FFlareTradeRouteSectorOperationSave* Operation) const
{
	return SelectedOperation != Operation ? false : true;
}

EVisibility SFlareTradeRouteMenu::GetQuantityLimitVisibility() const
{
	if (QuantityLimitButton->IsActive())
	{
		return EVisibility::Visible;
	}

	return EVisibility::Hidden;
}

EVisibility SFlareTradeRouteMenu::GetWaitLimitVisibility() const
{
	if (WaitLimitButton->IsActive())
	{
		return EVisibility::Visible;
	}

	return EVisibility::Hidden;
}


/*----------------------------------------------------
	Action callbacks
----------------------------------------------------*/

void SFlareTradeRouteMenu::OnResetStatistics()
{
	TargetTradeRoute->ResetStats();
}

void SFlareTradeRouteMenu::OnConfirmChangeRouteNameClicked()
{
	if (TargetTradeRoute)
	{
		TargetTradeRoute->SetTradeRouteName(EditRouteName->GetText());
	}
}

void SFlareTradeRouteMenu::OnAddSectorClicked()
{
	UFlareSimulatedSector* Item = SectorSelector->GetSelectedItem();
	if (Item)
	{
		TargetTradeRoute->AddSector(Item);
		OnAddOperationClicked(Item);
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnMoveLeft(UFlareSimulatedSector* Sector)
{
	TargetTradeRoute->MoveSectorUp(Sector);
	GenerateSectorList();
}

void SFlareTradeRouteMenu::OnMoveRight(UFlareSimulatedSector* Sector)
{
	TargetTradeRoute->MoveSectorDown(Sector);
	GenerateSectorList();
}

bool SFlareTradeRouteMenu::IsMoveLeftDisabled(UFlareSimulatedSector* Sector) const
{
	return (TargetTradeRoute->GetSectorIndex(Sector) == 0);
}

bool SFlareTradeRouteMenu::IsMoveRightDisabled(UFlareSimulatedSector* Sector) const
{
	return (TargetTradeRoute->GetSectorIndex(Sector) == TargetTradeRoute->GetSectors().Num() - 1);
}

void SFlareTradeRouteMenu::OnResourceComboLineSelectionChanged(UFlareResourceCatalogEntry* Item, ESelectInfo::Type SelectInfo)
{
	if (SelectedOperation)
	{
		SelectedOperation->ResourceIdentifier = Item->Data.Identifier;
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnOperationComboLineSelectionChanged(TSharedPtr<FText> Item, ESelectInfo::Type SelectInfo)
{
	if (SelectedOperation)
	{
		int32 OperationIndex  = OperationNameList.Find(Item);
		
		if (OperationIndex == -1)
		{
			OperationIndex = 0;
		}

		EFlareTradeRouteOperation::Type OperationType = OperationList[OperationIndex];
		SelectedOperation->Type = OperationType;
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnRemoveSectorClicked(UFlareSimulatedSector* Sector)
{
	if (TargetTradeRoute)
	{
		TargetTradeRoute->RemoveSector(Sector);
		OnDoneClicked();
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnAddOperationClicked(UFlareSimulatedSector* Sector)
{
	UFlareResourceCatalogEntry* Resource = ResourceSelector->GetSelectedItem();
	int32 OperationIndex  = OperationNameList.Find(OperationSelector->GetSelectedItem());

	if (OperationIndex == -1)
	{
		OperationIndex = 0;
	}
	
	int32 SectorIndex = TargetTradeRoute->GetSectorIndex(Sector);
	if (SectorIndex >= 0 && Resource)
	{
		EFlareTradeRouteOperation::Type OperationType = OperationList[OperationIndex];
		FFlareTradeRouteSectorOperationSave* Operation = TargetTradeRoute->AddSectorOperation(SectorIndex, OperationType, &Resource->Data);

		OnEditOperationClicked(Operation);
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnEditOperationClicked(FFlareTradeRouteSectorOperationSave* Operation)
{
	SelectedOperation = Operation;
	if (SelectedOperation)
	{
		FFlareResourceDescription* Resource = MenuManager->GetGame()->GetResourceCatalog()->Get(Operation->ResourceIdentifier);
		ResourceSelector->SetSelectedItem(MenuManager->GetGame()->GetResourceCatalog()->GetEntry(Resource));

		int32 OperationTypeIndex = OperationList.Find(Operation->Type);
		OperationSelector->SetSelectedItem(OperationNameList[OperationTypeIndex]);

		if (SelectedOperation->MaxQuantity == -1)
		{
			QuantityLimitButton->SetActive(false);
			QuantityLimitSlider->SetValue(0);
			QuantityLimitText->SetText(FText::AsNumber(0));
		}
		else
		{
			QuantityLimitButton->SetActive(true);
			if(TargetTradeRoute->GetFleet())
			{
				int32 Value = (float) (SelectedOperation->MaxQuantity - 1) / (float) TargetTradeRoute->GetFleet()->GetFleetCapacity();
				QuantityLimitSlider->SetValue(Value);
				QuantityLimitText->SetText(FText::AsNumber(Value));
			}
		}

		if (SelectedOperation->MaxWait == -1)
		{
			WaitLimitButton->SetActive(false);
			WaitLimitSlider->SetValue(0);
		}
		else
		{
			WaitLimitButton->SetActive(true);
			WaitLimitSlider->SetValue((float) (SelectedOperation->MaxWait -1 )/ (float) MAX_WAIT_LIMIT);
		}
	}
}

void SFlareTradeRouteMenu::OnDoneClicked()
{
	SelectedOperation = NULL;
}

void SFlareTradeRouteMenu::OnSkipOperationClicked()
{
	if (TargetTradeRoute)
	{
		TargetTradeRoute->SkipCurrentOperation();
	}
}

void SFlareTradeRouteMenu::OnPauseTradeRouteClicked()
{
	if (TargetTradeRoute)
	{
		if (TargetTradeRoute->IsPaused())
		{
			TargetTradeRoute->SetPaused(false);
		}
		else
		{
			TargetTradeRoute->SetPaused(true);
		}
	}
}

void SFlareTradeRouteMenu::OnDeleteOperationClicked(FFlareTradeRouteSectorOperationSave* Operation)
{
	if (Operation && TargetTradeRoute)
	{
		TargetTradeRoute->DeleteOperation(Operation);
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnOperationUpClicked()
{
	if (SelectedOperation && TargetTradeRoute)
	{
		SelectedOperation = TargetTradeRoute->MoveOperationUp(SelectedOperation);
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnOperationDownClicked()
{
	if (SelectedOperation && TargetTradeRoute)
	{
		SelectedOperation = TargetTradeRoute->MoveOperationDown(SelectedOperation);
		GenerateSectorList();
	}
}

void SFlareTradeRouteMenu::OnQuantityLimitToggle()
{
	if (SelectedOperation && TargetTradeRoute && TargetTradeRoute->GetFleet())
	{
		if (QuantityLimitButton->IsActive())
		{
			int32 Value = QuantityLimitSlider->GetValue() * TargetTradeRoute->GetFleet()->GetFleetCapacity();
			SelectedOperation->MaxQuantity = Value;
			QuantityLimitText->SetText(FText::AsNumber(Value));
		}
		else
		{
			SelectedOperation->MaxQuantity = -1;
		}
	}
}

void SFlareTradeRouteMenu::OnWaitLimitToggle()
{
	if (SelectedOperation && TargetTradeRoute)
	{
		if (WaitLimitButton->IsActive())
		{
			SelectedOperation->MaxWait = (WaitLimitSlider->GetValue() * MAX_WAIT_LIMIT) + 1;
		}
		else
		{
			SelectedOperation->MaxWait = -1;
		}
	}
}

void SFlareTradeRouteMenu::OnQuantityLimitChanged(float Value)
{
	if (SelectedOperation && TargetTradeRoute->GetFleet())
	{
		int32 NewValue = QuantityLimitSlider->GetValue() * TargetTradeRoute->GetFleet()->GetFleetCapacity();
		SelectedOperation->MaxQuantity = NewValue;
		QuantityLimitText->SetText(FText::AsNumber(NewValue));
	}
}

void SFlareTradeRouteMenu::OnQuantityLimitEntered(const FText& TextValue)
{
	if (TargetTradeRoute && SelectedOperation && TextValue.ToString().IsNumeric())
	{
		int32 ResourceMaxQuantity = 1000;

		if(TargetTradeRoute->GetFleet())
		{
			ResourceMaxQuantity = TargetTradeRoute->GetFleet()->GetFleetCapacity();
		}

		int32 TransactionQuantity = FMath::Clamp(FCString::Atoi(*TextValue.ToString()), 0, ResourceMaxQuantity);

		if (ResourceMaxQuantity == 1)
		{
			QuantityLimitSlider->SetValue(1.0f);
		}
		else
		{
			QuantityLimitSlider->SetValue((float)(TransactionQuantity - 1) / (float)(ResourceMaxQuantity - 1));
		}

		SelectedOperation->MaxQuantity = TransactionQuantity;
	}
}

void SFlareTradeRouteMenu::OnWaitLimitChanged(float Value)
{
	if (SelectedOperation)
	{
		SelectedOperation->MaxWait = (WaitLimitSlider->GetValue() * MAX_WAIT_LIMIT) + 1;
	}
}

void SFlareTradeRouteMenu::OnFleetComboLineSelectionChanged(UFlareFleet* Item, ESelectInfo::Type SelectInfo)
{
}

void SFlareTradeRouteMenu::OnAssignFleetClicked()
{
	UFlareFleet* Item = FleetSelector->GetSelectedItem();
	if (Item)
	{
		TargetTradeRoute->AssignFleet(Item);
		GenerateFleetList();
	}
}
void SFlareTradeRouteMenu::OnUnassignFleetClicked(UFlareFleet* Fleet)
{
	if (TargetTradeRoute)
	{
		TargetTradeRoute->RemoveFleet(Fleet);
		GenerateFleetList();
	}
}


#undef LOCTEXT_NAMESPACE
