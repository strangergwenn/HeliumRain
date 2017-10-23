
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

TMap<float, int32> SFlarePlanetaryBox::GetAltitudeStats() const
{
	TMap<float, int32> AltitudeStats;

	for (int32 ChildIndex = 1; ChildIndex < Children.Num(); ChildIndex++)
	{
		float Altitude = Children[ChildIndex].OrbitAltitude.Get();
		if (AltitudeStats.Find(Altitude) != NULL)
		{
			AltitudeStats[Altitude] = AltitudeStats[Altitude] + 1;
		}
		else
		{
			AltitudeStats.Add(Altitude, 1);
		}
	}

	AltitudeStats.KeySort([](float k1, float k2)
	{
		return (k1 < k2);
	});

	return AltitudeStats;
}

void SFlarePlanetaryBox::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	const FFlareStyleCatalog& Theme = FFlareStyleSet::GetDefaultTheme();

	// Iterate on orbits
	int OrbitIndex = 0;
	TMap<float, int> AltitudeStats = GetAltitudeStats();
	for (TPair<float, int> AltitudeData : AltitudeStats)
	{
		// Child with index 0 is the main body, index 1 is probably the name but we don't care, others are sectors
		int ValidChildIndex = 0;
		for (int32 ChildIndex = 1; ChildIndex < Children.Num(); ChildIndex++)
		{
			const SFlarePlanetaryBox::FSlot& CurChild = Children[ChildIndex];
			if (CurChild.OrbitAltitude.Get() != AltitudeData.Key)
			{
				continue;
			}

			// Get data
			float X, Y;
			float Angle = (360 / AltitudeData.Value) * ValidChildIndex - 90;
			float AltitudeRadius = Radius + RadiusIncrement * (OrbitIndex - AltitudeStats.Num() / 2);

			// Convert coordinates
			FMath::PolarToCartesian(AltitudeRadius, FMath::DegreesToRadians(Angle), X, Y);
			FVector2D Offset = FVector2D(X, Y);
			FVector2D WidgetSize = FVector2D(CurChild.GetWidget()->GetDesiredSize().X, CurChild.GetWidget()->GetDesiredSize().Y);
			
			// Arrange the child
			FVector2D Location = (AllottedGeometry.GetLocalSize() - WidgetSize) / 2 + Offset;
			ArrangedChildren.AddWidget(CurChild.GetWidget()->GetVisibility(), AllottedGeometry.MakeChild(
				CurChild.GetWidget(),
				Location,
				CurChild.GetWidget()->GetDesiredSize()
			));

			ValidChildIndex++;
		}
		OrbitIndex++;
	}

	// Draw planet
	const SFlarePlanetaryBox::FSlot& MainChild = Children[0];
	ArrangedChildren.AddWidget(MainChild.GetWidget()->GetVisibility(), AllottedGeometry.MakeChild(
		MainChild.GetWidget(),
		(AllottedGeometry.GetLocalSize() - MainChild.GetWidget()->GetDesiredSize()) / 2,
		MainChild.GetWidget()->GetDesiredSize()
	));
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
	float ExclusionHalfAngleTitle = 25;
	float ExclusionHalfAngleNormal = 15;
	FLinearColor Color = FLinearColor::White;
	Color.A = 0.3f;

	// Iterate on orbits
	int OrbitIndex = 0;
	TMap<float, int> AltitudeStats = GetAltitudeStats();
	for (TPair<float, int> AltitudeData : AltitudeStats)
	{
		TArray<SFlareOrbitSegment> Segments;
		float AltitudeRadius = Radius + RadiusIncrement * (OrbitIndex - AltitudeStats.Num() / 2);

		// Generate exclusion segments (segments where no orbit is drawn) and draw between them
		int ValidChildIndex = 0;
		for (int32 ChildIndex = 1; ChildIndex < Children.Num(); ChildIndex++)
		{
			float Angle = (360 / AltitudeData.Value) * ValidChildIndex - 90;
			float ExclusionHalfAngle = (ChildIndex == 1) ? ExclusionHalfAngleTitle : ExclusionHalfAngleNormal;

			if (Children[ChildIndex].OrbitAltitude.Get() == AltitudeData.Key)
			{
				Segments.Add(GenerateExclusionSegment(AltitudeRadius, Angle, ExclusionHalfAngle));
				ValidChildIndex++;
			}
		}
		OrbitIndex++;

		// Fill missing segments to get circle-looking ellipses
		if (Segments.Num() == 0)
		{
			Segments.Add(GenerateExclusionSegment(AltitudeRadius, 270));
		}
		if (Segments.Num() == 1)
		{
			Segments.Add(GenerateExclusionSegment(AltitudeRadius, 90));
		}
		if (Segments.Num() <= 2)
		{
			Segments.Insert(GenerateExclusionSegment(AltitudeRadius, 0), 1);
			Segments.Add(GenerateExclusionSegment(AltitudeRadius, 180));
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
				ESlateDrawEffect::NoPixelSnapping,
				Color);
		}
	}

	return SPanel::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

FVector2D SFlarePlanetaryBox::ComputeDesiredSize(float) const
{
	TMap<float, int> AltitudeStats = GetAltitudeStats();

	// Small hack to make Nema less humongous
	int AltitudeCount = GetAltitudeStats().Num();
	if (AltitudeCount > 1)
	{
		AltitudeCount -= 1;
	}

	float AltitudeRadius = Radius + RadiusIncrement * AltitudeCount;
	return FVector2D(2.8 * Radius, 2.8 * AltitudeRadius);
}

