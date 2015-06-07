
#include "../../Flare.h"
#include "FlareMouseMenu.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareMouseMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareMouseMenu::Construct(const FArguments& InArgs)
{
	// Setup
	WidgetDistance = 200;
	WidgetSize = 100;
	AnimTime = 0.10f;

	// Init
	OwnerHUD = InArgs._OwnerHUD;
	PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	SetVisibility(EVisibility::Hidden);
	CurrentTime = 0.0f;
	WidgetCount = 0;
	IsOpen = false;
	
	// Structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0))
	[
		SAssignNew(HUDCanvas, SCanvas)
	];
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareMouseMenu::AddWidget(FString Icon, FFlareMouseMenuClicked Action)
{
	// Update data
	int32 Index = WidgetCount;
	WidgetCount++;
	Actions.Add(Action);

	// Add widget
	HUDCanvas->AddSlot()
		.Position(TAttribute<FVector2D>::Create(TAttribute<FVector2D>::FGetter::CreateSP(this, &SFlareMouseMenu::GetWidgetPosition, Index)))
		.Size(TAttribute<FVector2D>::Create(TAttribute<FVector2D>::FGetter::CreateSP(this, &SFlareMouseMenu::GetWidgetSize, Index)))
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon(Icon))
			.ColorAndOpacity(this, &SFlareMouseMenu::GetWidgetColor, Index)
		];
}

void SFlareMouseMenu::ClearWidgets()
{
	HUDCanvas->ClearChildren();
	Actions.Empty();
}

void SFlareMouseMenu::Open()
{
	InitialMousePosition = PC->GetMousePosition();
	SetVisibility(EVisibility::HitTestInvisible);
	SetAnimDirection(true);
}

void SFlareMouseMenu::Close()
{
	// Result extraction
	if (HasSelection())
	{
		int32 Index = GetSelectedIndex();
		FLOGV("SFlareMouseMenu::Close : index %d", Index);
		Actions[Index].ExecuteIfBound();
	}

	// State data
	SetAnimDirection(false);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareMouseMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Viewport data
	FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	ViewportCenter = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);
	MouseOffset = PC->GetMousePosition() - InitialMousePosition;

	// Time data
	CurrentTime += IsOpen ? InDeltaTime : -InDeltaTime;
	if (!IsOpen && CurrentTime > AnimTime)
	{
		SetVisibility(EVisibility::Hidden);
	}
}

FVector2D SFlareMouseMenu::GetWidgetPosition(int32 Index) const
{
	return ViewportCenter - GetWidgetSize(Index) / 2 + GetDirection(Index);;
}

FVector2D SFlareMouseMenu::GetWidgetSize(int32 Index) const
{
	FVector2D BaseSize(WidgetSize, WidgetSize);
	float AnimAlpha = FMath::Clamp(CurrentTime / AnimTime, 0.0f, 1.0f);
	return BaseSize * AnimAlpha;
}

FSlateColor SFlareMouseMenu::GetWidgetColor(int32 Index) const
{
	FLinearColor Color = FLinearColor::White;
	float Colinearity = GetColinearity(Index);

	if (OwnerHUD->IsMenuOpen())
	{
		Color.A = 0;
	}
	else
	{
		Color.A = 0.3 + 0.7 * Colinearity;
	}

	return Color;
}


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

FVector2D SFlareMouseMenu::GetDirection(int32 Index) const
{
	return FVector2D(WidgetDistance, 0).GetRotated((Index * 360.0f) / (float)WidgetCount);
}

float SFlareMouseMenu::GetColinearity(int32 Index) const
{
	FVector2D MouseDirection = MouseOffset;
	MouseDirection.Normalize();

	FVector2D WidgetDirection = GetDirection(Index);
	WidgetDirection.Normalize();

	float Colinearity = FVector2D::DotProduct(MouseDirection, WidgetDirection);
	return FMath::Clamp(Colinearity, 0.0f, 1.0f);
}

bool SFlareMouseMenu::HasSelection() const
{
	return (MouseOffset).Size() > WidgetDistance / 2;
}

int32 SFlareMouseMenu::GetSelectedIndex() const
{
	int32 BestIndex = -1;
	float BestColinearity = -1;

	FVector2D MouseDirection = MouseOffset;
	MouseDirection.Normalize();

	for (int32 Index = 0; Index < WidgetCount; Index++)
	{
		FVector2D WidgetDirection = GetDirection(Index);
		WidgetDirection.Normalize();

		float Colinearity = FVector2D::DotProduct(MouseDirection, WidgetDirection);

		if (Colinearity > BestColinearity)
		{
			BestColinearity = Colinearity;
			BestIndex = Index;
		}
	}

	return BestIndex;
}

void SFlareMouseMenu::SetAnimDirection(bool Opening)
{
	IsOpen = Opening;
	if (CurrentTime < 0 || CurrentTime > AnimTime)
	{
		CurrentTime = Opening ? 0 : AnimTime;
	}
}
