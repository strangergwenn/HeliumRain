
#include "FlareMouseMenu.h"
#include "../../Flare.h"
#include "../Components/FlareRoundButton.h"
#include "../../Player/FlarePlayerController.h"

#include "Runtime/Engine/Classes/Engine/UserInterfaceSettings.h"
#include "Runtime/Engine/Classes/Engine/RendererSettings.h"


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
	AutoResetTime = 1.0f;
	Sensitivity = 10.0f;

	// Init
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();
	MenuManager = InArgs._MenuManager;
	PC = MenuManager->GetPC();
	SetVisibility(EVisibility::Collapsed);
	TimeSinceActive = 0.0f;
	CurrentTime = 0.0f;
	IsOpening = false;
	
	// Structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0))
	[
		SNew(SBackgroundBlur)
		.BlurRadius(Theme.BlurRadius)
		.BlurStrength(this, &SFlareMouseMenu::GetWheelMenuBlurStrength)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(FMargin(0))
		[
			SAssignNew(HUDCanvas, SCanvas)
		]
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
	TimeSinceActive = 0;
}

void SFlareMouseMenu::Close(bool EnableAction)
{
	// Result extraction
	if (HasSelection() && EnableAction)
	{
		if (SelectedIndex >= 0 && SelectedIndex < Actions.Num())
		{
			FLOGV("SFlareMouseMenu::Close : index %d", SelectedIndex);
			Actions[SelectedIndex].ExecuteIfBound();
			SelectedWidget = SelectedIndex;
			PC->ClientPlaySound(PC->GetSoundManager()->InfoSound);
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

bool SFlareMouseMenu::IsOpen() const
{
	return (CurrentTime > 0);
}

void SFlareMouseMenu::SetWheelCursorMove(FVector2D Move)
{
	if (Move != PreviousMove)
	{
		TimeSinceActive = 0;
		PreviousMove = Move;

		MouseOffset += Move * Sensitivity;
		if (MouseOffset.Size() > WidgetDistance)
		{
			MouseOffset /= MouseOffset.Size() / (float)WidgetDistance;
		}
	}
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
		SetVisibility(EVisibility::Hidden);
	}

	// Auto-reset
	else
	{
		TimeSinceActive += InDeltaTime;
		if (TimeSinceActive > AutoResetTime && MouseOffset != FVector2D::ZeroVector)
		{
			FLOG("SFlareMouseMenu::Tick : auto reset");
			MouseOffset = FVector2D::ZeroVector;
		}
	}

	// Selection
	int32 PreviousSelectedIndex = SelectedIndex;
	if (HasSelection())
	{
		int32 BestIndex = -1;
		float BestColinearity = -1;
		FVector2D MouseDirection = MouseOffset;
		MouseDirection.Normalize();

		// Find the best index
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

		SelectedIndex = BestIndex;
	}
	else
	{
		SelectedIndex = -1;
	}

	// Switch sound
	if (SelectedIndex != PreviousSelectedIndex)
	{
		PC->ClientPlaySound(PC->GetSoundManager()->TickSound);
	}
}

float SFlareMouseMenu::GetWheelMenuBlurStrength() const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	bool ShowBlur;
	if (IsOpening)
	{
		ShowBlur = (CurrentTime > 0);
	}
	else
	{
		ShowBlur = (CurrentTime >= 1);
	}

	return ShowBlur ? Theme.BlurStrength  : 0;
}

FVector2D SFlareMouseMenu::GetWidgetPosition(int32 Index) const
{
	FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	float ViewportScale = GetDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass())->GetDPIScaleBasedOnSize(FIntPoint(ViewportSize.X, ViewportSize.Y));

	return (ViewportCenter + GetDirection(Index)) / ViewportScale;
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
	float AnimAlpha = FMath::Clamp(CurrentTime / AnimTime, 0.0f, 1.0f);
	
	// Menu open, maks everything
	if (MenuManager->IsUIOpen())
	{
		Color.A = 0;
	}

	// Selected item
	else if (Index == SelectedIndex)
	{
		Color = Theme.FriendlyColor;
		Color.A = AnimAlpha * Theme.DefaultAlpha;
	}

	// Regular item
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

void SFlareMouseMenu::SetAnimDirection(bool Opening)
{
	IsOpening = Opening;
	if (CurrentTime < 0 || CurrentTime > AnimTime)
	{
		CurrentTime = Opening ? 0 : AnimTime;
	}
}

#undef LOCTEXT_NAMESPACE
