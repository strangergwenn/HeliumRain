
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
	WidgetSize = 200;
	AnimTime = 0.20f;
	ColinearityPower = 4.0f;

	// Init
	MenuManager = InArgs._MenuManager;
	PC = MenuManager->GetPC();
	SetVisibility(EVisibility::Collapsed);
	CurrentTime = 0.0f;
	IsOpening = false;
	
	// Structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0))
	[
		SAssignNew(HUDCanvas, SCanvas)
	];

	SetVisibility(EVisibility::Collapsed);
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareMouseMenu::AddWidget(FString Icon, FText Legend, FFlareMouseMenuClicked Action)
{
	int32 Index = Actions.Num();
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
	MouseOffset = FVector2D::ZeroVector;
	SetVisibility(EVisibility::HitTestInvisible);
	SetAnimDirection(true);
}

void SFlareMouseMenu::Close(bool EnableAction)
{
	// Result extraction
	if (HasSelection() && EnableAction)
	{
		int32 Index = GetSelectedIndex();
		if (Index >= 0 && Index < Actions.Num())
		{
			FLOGV("SFlareMouseMenu::Close : index %d", Index);
			Actions[Index].ExecuteIfBound();
			SelectedWidget = Index;
		}
	}
	else
	{
		FLOG("SFlareMouseMenu::Close : no action taken");
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

	// Time data
	CurrentTime += IsOpening ? InDeltaTime : -InDeltaTime;
	if (!IsOpen())
	{
		SetVisibility(EVisibility::Collapsed);
	}
}

void SFlareMouseMenu::SetWheelCursorMove(FVector2D Move)
{
	MouseOffset += Move * 15; // Wheel menu sensibility
	if (MouseOffset.Size() > WidgetDistance)
	{
		MouseOffset /= MouseOffset.Size() / (float) WidgetDistance;
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

	if (MenuManager->IsUIOpen())
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
		if (SelectedWidget < Actions.Num())
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
		OperatorAlpha = FMath::Pow(OperatorAlpha, ColinearityPower);

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
			SNew(SBorder)
			.BorderImage(FFlareStyleSet::GetIcon("LargeButtonBackground"))
			.BorderBackgroundColor(this, &SFlareMouseMenu::GetWidgetColor, Index)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SFlareRoundButton)
				.Clickable(false)
				.Text(Legend)
				.Icon(FFlareStyleSet::GetIcon(Icon))
				.HighlightColor(this, &SFlareMouseMenu::GetWidgetColor, Index)
				.IconColor(this, &SFlareMouseMenu::GetWidgetColor, Index)
				.TextColor(this, &SFlareMouseMenu::GetWidgetColor, Index)
			]
		];
}

FVector2D SFlareMouseMenu::GetDirection(int32 Index) const
{
	if (Index >= 0)
	{
		return FVector2D(0, -WidgetDistance).GetRotated((Index * 360.0f) / (float)Actions.Num());
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

	for (int32 Index = 0; Index < Actions.Num(); Index++)
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
