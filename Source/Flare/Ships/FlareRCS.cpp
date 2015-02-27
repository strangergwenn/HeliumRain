
#include "../Flare.h"
#include "FlareRCS.h"
#include "FlareShip.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareRCS::UFlareRCS(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}


/*----------------------------------------------------
	Gameplay
----------------------------------------------------*/

bool UFlareRCS::CanMoveVertical()
{
	for (auto Iter = Capabilities.CreateIterator(); Iter; Iter++)
	{
		if (*Iter == ERCSCapability::StrafeUp || *Iter == ERCSCapability::StrafeDown)
		{
			return true;
		}
	}
	return false;
}

bool UFlareRCS::CanMoveHorizontal()
{
	for (auto Iter = Capabilities.CreateIterator(); Iter; Iter++)
	{
		if (*Iter == ERCSCapability::StrafeRight || *Iter == ERCSCapability::StrafeLeft)
		{
			return true;
		}
	}
	return false;
}
