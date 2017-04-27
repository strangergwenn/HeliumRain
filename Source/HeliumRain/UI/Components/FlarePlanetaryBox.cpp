
#include "FlarePlanetaryBox.h"
#include "../../Flare.h"


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

FVector2D SFlarePlanetaryBox::ComputeDesiredSize(float) const
{
	return FVector2D(2.6 * Radius, 2.8 * Radius);
}

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
