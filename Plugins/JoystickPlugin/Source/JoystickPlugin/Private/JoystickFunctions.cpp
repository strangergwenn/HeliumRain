
#include "JoystickFunctions.h"
#include <Engine.h>
#include "IJoystickPlugin.h"
#include "JoystickInterface.h"
#include "JoystickDevice.h"
#include "JoystickPlugin.h"

UJoystickFunctions::UJoystickFunctions(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{

}

FVector2D UJoystickFunctions::POVAxis(EJoystickPOVDirection Direction)
{
	return ::POVAxis(Direction);
}

FJoystickInfo UJoystickFunctions::GetJoystick(int32 DeviceId)
{
	if (!IJoystickPlugin::IsAvailable())
		return FJoystickInfo();

	TSharedPtr<FJoystickDevice> Device = static_cast<FJoystickPlugin&>(IJoystickPlugin::Get()).JoystickDevice;
	if (!Device->InputDevices.Contains(FDeviceId(DeviceId)))
		return FJoystickInfo();

	return Device->InputDevices[FDeviceId(DeviceId)];
}

FJoystickState UJoystickFunctions::GetJoystickState(int32 DeviceId)
{
	if (!IJoystickPlugin::IsAvailable())
		return FJoystickState();

	TSharedPtr<FJoystickDevice> Device = static_cast<FJoystickPlugin&>(IJoystickPlugin::Get()).JoystickDevice;
	if (!Device->InputDevices.Contains(FDeviceId(DeviceId)))
		return FJoystickState();

	return Device->CurrentState[FDeviceId(DeviceId)];
}

FJoystickState UJoystickFunctions::GetPreviousJoystickState(int32 DeviceId)
{
	if (!IJoystickPlugin::IsAvailable())
		return FJoystickState();

	TSharedPtr<FJoystickDevice> Device = static_cast<FJoystickPlugin&>(IJoystickPlugin::Get()).JoystickDevice;
	if (!Device->InputDevices.Contains(FDeviceId(DeviceId)))
		return FJoystickState();

	return Device->PreviousState[FDeviceId(DeviceId)];
}

int32 UJoystickFunctions::JoystickCount()
{
	if (!IJoystickPlugin::IsAvailable())
		return 0;

	TSharedPtr<FJoystickDevice> Device = static_cast<FJoystickPlugin&>(IJoystickPlugin::Get()).JoystickDevice;
	return Device->InputDevices.Num();
}

void UJoystickFunctions::RegisterForJoystickEvents(UObject* Listener)
{
	if (!IJoystickPlugin::IsAvailable())
		return;

	TSharedPtr<FJoystickDevice> Device = static_cast<FJoystickPlugin&>(IJoystickPlugin::Get()).JoystickDevice;
	Device->AddEventListener(Listener);
}

void UJoystickFunctions::MapJoystickDeviceToPlayer(int32 DeviceId, int32 Player)
{
	if (!IJoystickPlugin::IsAvailable())
		return;

	TSharedPtr<FJoystickDevice> Device = static_cast<FJoystickPlugin&>(IJoystickPlugin::Get()).JoystickDevice;
	if (!Device->InputDevices.Contains(FDeviceId(DeviceId)))
		return;

	Device->InputDevices[FDeviceId(DeviceId)].Player = Player;
}

void UJoystickFunctions::IgnoreGameControllers(bool bIgnore)
{
	if (!IJoystickPlugin::IsAvailable()) return;

	TSharedPtr<FJoystickDevice> Device = static_cast<FJoystickPlugin&>(IJoystickPlugin::Get()).JoystickDevice;
	Device->IgnoreGameControllers(bIgnore);
}
