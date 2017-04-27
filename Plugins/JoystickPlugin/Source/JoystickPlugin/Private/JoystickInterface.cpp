
#include "JoystickInterface.h"
#include <Engine.h>

UJoystickInterface::UJoystickInterface(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{

}

//This is required for compiling, would also let you know if somehow you called
//the base event/function rather than the over-rided version
FString IJoystickInterface::ToString()
{
	return "IJoystickInterface::ToString()";
}

FVector2D POVAxis(EJoystickPOVDirection povValue)
{
	switch (povValue){
	case EJoystickPOVDirection::DIRECTION_NONE:
		return FVector2D(0, 0);
	case EJoystickPOVDirection::DIRECTION_UP:
		return FVector2D(0, 1);
	case EJoystickPOVDirection::DIRECTION_UP_RIGHT:
		return FVector2D(1, 1);
	case EJoystickPOVDirection::DIRECTION_RIGHT:
		return FVector2D(1, 0);
	case EJoystickPOVDirection::DIRECTION_DOWN_RIGHT:
		return FVector2D(1, -1);
	case EJoystickPOVDirection::DIRECTION_DOWN:
		return FVector2D(0, -1);
	case EJoystickPOVDirection::DIRECTION_DOWN_LEFT:
		return FVector2D(-1, -1);
	case EJoystickPOVDirection::DIRECTION_LEFT:
		return FVector2D(-1, 0);
	case EJoystickPOVDirection::DIRECTION_UP_LEFT:
		return FVector2D(-1, 1);
	default:
		return FVector2D(0, 0);
	}
}
