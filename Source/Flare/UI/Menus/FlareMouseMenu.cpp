
#include "../../Flare.h"
#include "FlareMouseMenu.h"
#include "../Components/FlareRoundButton.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareMouseMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareMouseMenu::Construct(const FArguments& InArgs)
{
	// Setup
	WidgetDistance = 192;
	WidgetSize = 150;
	AnimTime = 0.20f;

	// Init
	OwnerHUD = InArgs._OwnerHUD;
	PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	SetVisibility(EVisibility::Hidden);
	CurrentTime = 0.0f;
	WidgetCount = 0;
	IsOpening = false;
	
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

void SFlareMouseMenu::AddWidget(FString Icon, FText Legend, FFlareMouseMenuClicked Action)
{
	int32 Index = WidgetCount;
	WidgetCount++;
	Actions.Add(Action);

	AddWidgetInternal(Icon, Legend, Index);
}

void SFlareMouseMenu::AddDefaultWidget(FString Icon, FText Legend)
{
	AddWidgetInternal(Icon, Legend, -1);
}

void SFlareMouseMenu::AddDefaultWidget(FString Icon, FText Legend, FFlareMouseMenuClicked Action)
{
	DefaultAction = Action;
	AddWidgetInternal(Icon, Legend, -1);
}

void SFlareMouseMenu::ClearWidgets()
{
	HUDCanvas->ClearChildren();
	Actions.Empty();
	DefaultAction.Unbind();
}

void SFlareMouseMenu::Open()
{
	SelectedWidget = 1000; // Arbitrary large value
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
		SelectedWidget = Index;
	}
	else
	{
		FLOGV("SFlareMouseMenu::Close : no action taken");
		DefaultAction.ExecuteIfBound();
		SelectedWidget = -1;
	}

	// State data
	SetAnimDirection(false);
}

bool SFlareMouseMenu::IsOpen()
{
	return (CurrentTime > 0);
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
	CurrentTime += IsOpening ? InDeltaTime : -InDeltaTime;
	if (!IsOpen())
	{
		SetVisibility(EVisibility::Hidden);
	}
}

FVector2D SFlareMouseMenu::GetWidgetPosition(int32 Index) const
{
	return ViewportCenter + GetDirection(Index);
}

FVector2D SFlareMouseMenu::GetWidgetSize(int32 Index) const
{
	FVector2D BaseSize(WidgetSize, WidgetSize);
	return BaseSize;
}

FSlateColor SFlareMouseMenu::GetWidgetColor(int32 Index) const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	FLinearColor Color = Theme.NeutralColor;

	if (OwnerHUD->IsMenuOpen())
	{
		Color.A = 0;
	}
	else
	{
		// Compute basic data
		float Colinearity = GetColinearity(Index);
		float DistanceRatio = FMath::Clamp(2 * (MouseOffset.Size() / WidgetDistance - 0.5f), 0.0f, 1.0f);
		if (Index < 0)
		{
			DistanceRatio = 1.0f - DistanceRatio;
			Colinearity = 1.0f;
		}

		// Compute the alpha based on the widget's position and state
		float OperatorAlpha = Colinearity * DistanceRatio;
		if (SelectedWidget < WidgetCount)
		{
			if (Index == SelectedWidget)
			{
				OperatorAlpha = 1.0f;
			}
			else
			{
				OperatorAlpha = 0.0f;
			}
		}

		// Update the alpha to account for animation
		float AnimAlpha = FMath::Clamp(CurrentTime / AnimTime, 0.0f, 1.0f);
		Color.A = AnimAlpha * ((1.0f - Theme.DefaultAlpha) + Theme.DefaultAlpha * OperatorAlpha);
	}

	return Color;
}


/*----------------------------------------------------
	Helpers
----------------------------------------------------*/

void SFlareMouseMenu::AddWidgetInternal(FString Icon, FText Legend, int32 Index)
{
	HUDCanvas->AddSlot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Position(TAttribute<FVector2D>::Create(TAttribute<FVector2D>::FGetter::CreateSP(this, &SFlareMouseMenu::GetWidgetPosition, Index)))
		.Size(TAttribute<FVector2D>::Create(TAttribute<FVector2D>::FGetter::CreateSP(this, &SFlareMouseMenu::GetWidgetSize, Index)))
		[
			SNew(SFlareRoundButton)
			.Clickable(false)
			.Text(Legend)
			.Icon(FFlareStyleSet::GetIcon(Icon))
			.HighlightColor(this, &SFlareMouseMenu::GetWidgetColor, Index)
			.IconColor(this, &SFlareMouseMenu::GetWidgetColor, Index)
			.TextColor(this, &SFlareMouseMenu::GetWidgetColor, Index)
		];
}

FVector2D SFlareMouseMenu::GetDirection(int32 Index) const
{
	if (Index >= 0)
	{
		return FVector2D(0, -WidgetDistance).GetRotated((Index * 360.0f) / (float)WidgetCount);
	}
	else
	{
		return FVector2D::ZeroVector;
	}
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
	IsOpening = Opening;
	if (CurrentTime < 0 || CurrentTime > AnimTime)
	{
		CurrentTime = Opening ? 0 : AnimTime;
	}
}

#undef LOCTEXT_NAMESPACE
