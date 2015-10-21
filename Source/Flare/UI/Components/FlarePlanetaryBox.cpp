
#include "../../Flare.h"
#include "FlarePlanetaryBox.h"



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
	if (Children.Num() > 1)
	{
		FVector2D TypicalSize = Children[1].GetWidget()->GetDesiredSize();

		for (int32 ChildIndex = 0; ChildIndex < Children.Num(); ++ChildIndex)
		{
			// Get all required data
			const SFlarePlanetaryBox::FSlot& CurChild = Children[ChildIndex];
			const EVisibility ChildVisibility = CurChild.GetWidget()->GetVisibility();
			FVector2D Origin = (AllottedGeometry.GetLocalSize() - CurChild.GetWidget()->GetDesiredSize()) / 2;
			FVector2D Offset = FVector2D::ZeroVector;

			// Child with index 0 is the main body, index 1 is probably the name but we don't care, others are sectors
			if (ChildIndex > 0)
			{
				float X, Y;
				float Angle = (360 / (Children.Num() - 1)) * (ChildIndex - 1) - 90;
				FMath::PolarToCartesian(Radius, FMath::DegreesToRadians(Angle), X, Y);
				Offset =  FVector2D(X, Y);
			}
			
			// Arrange the child
			ArrangedChildren.AddWidget(ChildVisibility, AllottedGeometry.MakeChild(
				CurChild.GetWidget(),
				Origin + Offset,
				CurChild.GetWidget()->GetDesiredSize()
			));
		}
	}
}

/**
* Helper to ComputeDesiredSize().
*
* @param Orientation   Template parameters that controls the orientation in which the children are layed out
* @param Children      The children whose size we want to assess in a horizontal or vertical arrangement.
*
* @return The size desired by the children given an orientation.
*/
template<EOrientation Orientation>
static FVector2D ComputeDesiredSizeForBox(const TPanelChildren<SFlarePlanetaryBox::FSlot>& Children)
{
	// The desired size of this panel is the total size desired by its children plus any margins specified in this panel.
	// The layout along the panel's axis is describe dy the SizeParam, while the perpendicular layout is described by the
	// alignment property.
	FVector2D MyDesiredSize(0, 0);
	for (int32 ChildIndex = 0; ChildIndex < Children.Num(); ++ChildIndex)
	{
		const SFlarePlanetaryBox::FSlot& CurChild = Children[ChildIndex];
		const FVector2D& CurChildDesiredSize = CurChild.GetWidget()->GetDesiredSize();
		if (CurChild.GetWidget()->GetVisibility() != EVisibility::Collapsed)
		{
			if (Orientation == Orient_Vertical)
			{
				// For a vertical panel, we want to find the maximum desired width (including margin).
				// That will be the desired width of the whole panel.
				MyDesiredSize.X = FMath::Max(MyDesiredSize.X, CurChildDesiredSize.X + CurChild.SlotPadding.Get().GetTotalSpaceAlong<Orient_Horizontal>());

				// Clamp to the max size if it was specified
				float FinalChildDesiredSize = CurChildDesiredSize.Y;
				float MaxSize = CurChild.MaxSize.Get();
				if (MaxSize > 0)
				{
					FinalChildDesiredSize = FMath::Min(MaxSize, FinalChildDesiredSize);
				}

				MyDesiredSize.Y += FinalChildDesiredSize + CurChild.SlotPadding.Get().GetTotalSpaceAlong<Orient_Vertical>();
			}
			else
			{
				// A horizontal panel is just a sideways vertical panel: the axes are swapped.

				MyDesiredSize.Y = FMath::Max(MyDesiredSize.Y, CurChildDesiredSize.Y + CurChild.SlotPadding.Get().GetTotalSpaceAlong<Orient_Vertical>());

				// Clamp to the max size if it was specified
				float FinalChildDesiredSize = CurChildDesiredSize.X;
				float MaxSize = CurChild.MaxSize.Get();
				if (MaxSize > 0)
				{
					FinalChildDesiredSize = FMath::Min(MaxSize, FinalChildDesiredSize);
				}

				MyDesiredSize.X += FinalChildDesiredSize + CurChild.SlotPadding.Get().GetTotalSpaceAlong<Orient_Horizontal>();
			}
		}
	}

	return MyDesiredSize;
}

FVector2D SFlarePlanetaryBox::ComputeDesiredSize(float) const
{
	return ComputeDesiredSizeForBox<Orient_Horizontal>(this->Children);
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
