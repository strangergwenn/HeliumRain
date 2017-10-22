
#include "FlarePlanetaryBox.h"
#include "../../Flare.h"
#include "../Style/FlareStyleSet.h"


/*----------------------------------------------------
	Construction
----------------------------------------------------*/

void SFlarePlanetaryBox::Construct(const SFlarePlanetaryBox::FArguments& InArgs)
{
	Radius = 200;
	PlanetImage = NULL;
	ClearChildren();

	// Add slots
	const int32 NumSlots = InArgs.Slots.Num();
	for (int32 SlotIndex = 0; SlotIndex < NumSlots; ++SlotIndex)
	{
		Children.Add(InArgs.Slots[SlotIndex]);
	}
}


/*----------------------------------------------------
	Interaction
----------------------------------------------------*/

FChildren* SFlarePlanetaryBox::GetChildren()
{
	return &Children;
}

int32 SFlarePlanetaryBox::RemoveSlot(const TSharedRef<SWidget>& SlotWidget)
{
	for (int32 SlotIdx = 0; SlotIdx < Children.Num(); ++SlotIdx)
	{
		if (SlotWidget == Children[SlotIdx].GetWidget())
		{
			Children.RemoveAt(SlotIdx);
			return SlotIdx;
		}
	}

	return -1;
}

void SFlarePlanetaryBox::ClearChildren()
{
	Children.Empty();

	if (PlanetImage)
	{
		AddSlot()
			[
				SNew(SImage).Image(PlanetImage)
			];
	}
}


/*----------------------------------------------------
	Drawing
----------------------------------------------------*/

void SFlarePlanetaryBox::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	for (int32 ChildIndex = 0; ChildIndex < Children.Num(); ++ChildIndex)
	{
		// Get all required data
		const SFlarePlanetaryBox::FSlot& CurChild = Children[ChildIndex];
		const EVisibility ChildVisibility = CurChild.GetWidget()->GetVisibility();
		FVector2D WidgetSize = CurChild.GetWidget()->GetDesiredSize();
		FVector2D Offset = FVector2D::ZeroVector;

		// Child with index 0 is the main body, index 1 is probably the name but we don't care, others are sectors
		if (ChildIndex > 0)
		{
			float X, Y;
			float Angle = (360 / (Children.Num() - 1)) * (ChildIndex - 1) - 90;
			FMath::PolarToCartesian(Radius, FMath::DegreesToRadians(Angle), X, Y);
			Offset = FVector2D(X, Y);
			WidgetSize = FVector2D(CurChild.GetWidget()->GetDesiredSize().X, Theme.SectorButtonHeight);
		}
			
		// Arrange the child
		FVector2D Location = (AllottedGeometry.GetLocalSize() - WidgetSize) / 2 + Offset;
		ArrangedChildren.AddWidget(ChildVisibility, AllottedGeometry.MakeChild(
			CurChild.GetWidget(),
			Location,
			CurChild.GetWidget()->GetDesiredSize()
		));
	}
}

struct SFlareOrbitSegment
{
	FVector2D Start;
	FVector2D End;
	FVector2D StartTangent;
	FVector2D EndTangent;
	float CenterAngle;
	float ExclusionHalfAngle;
};

SFlareOrbitSegment GenerateExclusionSegment(float Radius, float Angle, float ExclusionHalfAngle = 0)
{
	SFlareOrbitSegment Segment;
	Segment.CenterAngle = Angle;
	Segment.ExclusionHalfAngle = ExclusionHalfAngle;

	float X, Y;
	FMath::PolarToCartesian(Radius, FMath::DegreesToRadians(Angle - ExclusionHalfAngle), X, Y);
	Segment.Start.X = X;
	Segment.Start.Y = Y;
	Segment.StartTangent.X = -Y;
	Segment.StartTangent.Y = X;

	FMath::PolarToCartesian(Radius, FMath::DegreesToRadians(Angle + ExclusionHalfAngle), X, Y);
	Segment.End.X = X;
	Segment.End.Y = Y;
	Segment.EndTangent.X = -Y;
	Segment.EndTangent.Y = X;

	return Segment;
}

int32 SFlarePlanetaryBox::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyClippingRect, 
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	// Parameters
	float ExclusionHalfAngleTitle = 20;
	float ExclusionHalfAngleNormal = 10;
	FLinearColor Color = FLinearColor::White;
	Color.A = 0.15f;

	// Get all altitudes
	TArray<float> Altitudes;
	for (int32 ChildIndex = 1; ChildIndex < Children.Num(); ChildIndex++)
	{
		Altitudes.AddUnique(Children[ChildIndex].OrbitAltitude.Get());
	}

	// Generate exclusion segments (segments where no orbit is drawn) and draw between them
	TArray<SFlareOrbitSegment> Segments;
	for (int32 ChildIndex = 1; ChildIndex < Children.Num(); ChildIndex++)
	{
		float Angle = (360 / (Children.Num() - 1)) * (ChildIndex - 1) - 90;
		float ExclusionHalfAngle = (ChildIndex == 1) ? ExclusionHalfAngleTitle : ExclusionHalfAngleNormal;
		Segments.Add(GenerateExclusionSegment(Radius, Angle, ExclusionHalfAngle));
	}

	// Fill missing segments to get circle-looking ellipses
	if (Segments.Num() == 0)
	{
		Segments.Add(GenerateExclusionSegment(Radius, 270));
	}
	if (Segments.Num() == 1)
	{
		Segments.Add(GenerateExclusionSegment(Radius, 90));
	}
	if (Segments.Num() <= 2)
	{
		Segments.Insert(GenerateExclusionSegment(Radius, 0), 1);
		Segments.Add(GenerateExclusionSegment(Radius, 180));
	}

	// Draw orbit segments between excluded segments
	for (int32 CurrentIndex = 0; CurrentIndex < Segments.Num(); CurrentIndex++)
	{
		int NextIndex = (CurrentIndex + 1) % Segments.Num();
		FVector2D CircleCenter = AllottedGeometry.GetLocalSize() / 2;

		// Generate tangent length from angle delta
		float ArcDelta = Segments[NextIndex].CenterAngle - Segments[CurrentIndex].CenterAngle;
		float ExclusionDelta = Segments[CurrentIndex].ExclusionHalfAngle + Segments[NextIndex].ExclusionHalfAngle;
		float ArcLength = (PI / 2) * (ArcDelta - ExclusionDelta) / 90;
		if (CurrentIndex == Segments.Num() - 1)
		{
			ArcLength += 2 * PI;
		}

		// Draw
		FSlateDrawElement::MakeSpline(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			CircleCenter + Segments[CurrentIndex].End,
			ArcLength * Segments[CurrentIndex].EndTangent,
			CircleCenter + Segments[NextIndex].Start,
			ArcLength * Segments[NextIndex].StartTangent,
			1,
			ESlateDrawEffect::None,
			Color);
	}

	return SPanel::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

FVector2D SFlarePlanetaryBox::ComputeDesiredSize(float) const
{
	return FVector2D(2.6 * Radius, 2.8 * Radius);
}

