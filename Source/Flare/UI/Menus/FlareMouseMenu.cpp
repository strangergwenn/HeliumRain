
#include "../../Flare.h"
#include "FlareMouseMenu.h"
#include "../../Player/FlarePlayerController.h"


#define LOCTEXT_NAMESPACE "FlareMouseMenu"


/*----------------------------------------------------
	Construct
----------------------------------------------------*/

void SFlareMouseMenu::Construct(const FArguments& InArgs)
{
	// Data
	OwnerHUD = InArgs._OwnerHUD;
	PC = Cast<AFlarePlayerController>(OwnerHUD->GetOwner());
	WidgetCount = 0;
	WidgetDistance = 200;
	WidgetSize = 100;
	SetVisibility(EVisibility::Hidden);
	
	// Structure
	ChildSlot
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0))
	[
		SAssignNew(HUDCanvas, SCanvas)
	];

	// Layout
	AddWidget();
	AddWidget();
	AddWidget();
	AddWidget();
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

void SFlareMouseMenu::AddWidget()
{
	// Update data
	int32 Index = WidgetCount;
	WidgetCount++;
	// TODO arbitrary image and callback store

	// Add widget
	HUDCanvas->AddSlot()
		.Position(TAttribute<FVector2D>::Create(TAttribute<FVector2D>::FGetter::CreateSP(this, &SFlareMouseMenu::GetWidgetPosition, Index)))
		.Size(TAttribute<FVector2D>::Create(TAttribute<FVector2D>::FGetter::CreateSP(this, &SFlareMouseMenu::GetWidgetSize, Index)))
		[
			SNew(SImage)
			.Image(FFlareStyleSet::GetIcon("HUD_Power"))
			.ColorAndOpacity(this, &SFlareMouseMenu::GetWidgetColor, Index)
		];
}

void SFlareMouseMenu::Open()
{
	FLOG("SFlareMouseMenu::Open");
	InitialMousePosition = PC->GetMousePosition();

	SetVisibility(EVisibility::HitTestInvisible);
}

void SFlareMouseMenu::Close()
{
	if (HasSelection())
	{
		int32 Index = GetSelectedIndex();
		FLOGV("SFlareMouseMenu::Close : index %d", Index);
		// TODO insert action here depending on Index
	}

	SetVisibility(EVisibility::Hidden);
}


/*----------------------------------------------------
	Callbacks
----------------------------------------------------*/

void SFlareMouseMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	ViewportCenter = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);

	MouseOffset = PC->GetMousePosition() - InitialMousePosition;
}

FVector2D SFlareMouseMenu::GetWidgetPosition(int32 Index) const
{
	FVector2D WidgetDirection = GetDirection(Index);
	FVector2D ConstantOffset = FVector2D(-WidgetSize, -WidgetSize) / 2;
	return ViewportCenter + ConstantOffset + WidgetDirection;
}

FVector2D SFlareMouseMenu::GetWidgetSize(int32 Index) const
{
	return FVector2D(WidgetSize, WidgetSize);
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
