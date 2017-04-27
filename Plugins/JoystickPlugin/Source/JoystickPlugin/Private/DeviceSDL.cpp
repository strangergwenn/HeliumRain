/*
*
* Copyright (C) <2014> samiljan <Sam Persson>, tsky <thomas.kollakowksy@w-hs.de>
* All rights reserved.
*
* This software may be modified and distributed under the terms
* of the BSD license.  See the LICENSE file for details.
*/

#include "DeviceSDL.h"
#include <Engine.h>
#include "JoystickDevice.h"

// @third party code - BEGIN SDL
#include "SDL.h"
#include "SDL_joystick.h"
#include "SDL_gamecontroller.h"
// @third party code - END SDL

DEFINE_LOG_CATEGORY(JoystickPluginLog);

FDeviceSDL::FDeviceSDL(IJoystickEventInterface * eventInterface) 
	: EventInterface(eventInterface)
{
}

void FDeviceSDL::Init()
{ 
	UE_LOG(JoystickPluginLog, Log, TEXT("DeviceSDL Starting"));

	if (SDL_WasInit(0) != 0)
	{
		UE_LOG(JoystickPluginLog, Log, TEXT("SDL already loaded"));
		bOwnsSDL = false;
	}
	else
	{
		UE_LOG(JoystickPluginLog, Log, TEXT("DeviceSDL::InitSDL() SDL init 0"));
		SDL_Init(0);
		bOwnsSDL = true;
	}

	int result = SDL_InitSubSystem(SDL_INIT_HAPTIC);
	if (result == 0)
	{
		UE_LOG(JoystickPluginLog, Log, TEXT("DeviceSDL::InitSDL() SDL init subsystem haptic"));
	}

	result = SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
	if (result == 0)
	{
		UE_LOG(JoystickPluginLog, Log, TEXT("DeviceSDL::InitSDL() SDL init subsystem joystick"));
	}

	result = SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	if (result == 0)
	{
		UE_LOG(JoystickPluginLog, Log, TEXT("DeviceSDL::InitSDL() SDL init subsystem joystick"));
	}

	int Joysticks = SDL_NumJoysticks();
	for (int i = 0; i < Joysticks; i++)
	{
		AddDevice(FDeviceIndex(i));
	}

	SDL_AddEventWatch(HandleSDLEvent, this);
}

FDeviceSDL::~FDeviceSDL()
{
	UE_LOG(JoystickPluginLog, Log, TEXT("DeviceSDL Closing"));

	for (auto & Device : Devices)
	{
		RemoveDevice(Device.Key);
	}

	SDL_DelEventWatch(HandleSDLEvent, this);

	if (bOwnsSDL)
	{
		SDL_Quit();
	}
}

FDeviceInfoSDL * FDeviceSDL::GetDevice(FDeviceId DeviceId)
{
	if (Devices.Contains(DeviceId))
	{
		return &Devices[DeviceId];
	}
	return nullptr;
}

void FDeviceSDL::IgnoreGameControllers(bool bIgnore)
{
	if (bIgnore && !bIgnoreGameControllers)
	{
		bIgnoreGameControllers = true;
		for (auto &Device : Devices)
		{
			if (DeviceMapping.Contains(Device.Value.InstanceId) && SDL_IsGameController(Device.Value.DeviceIndex.value))
			{
				RemoveDevice(Device.Key);
			}
		}
	}
	else if (!bIgnore && bIgnoreGameControllers)
	{
		bIgnoreGameControllers = false;
		int Joysticks = SDL_NumJoysticks();
		for (int i = 0; i < Joysticks; i++)
		{
			if (SDL_IsGameController(i))
			{
				AddDevice(FDeviceIndex(i));
			}
		}
	}
}

FDeviceInfoSDL FDeviceSDL::AddDevice(FDeviceIndex DeviceIndex)
{
	FDeviceInfoSDL Device;
	if (SDL_IsGameController(DeviceIndex.value) && bIgnoreGameControllers)
	{
		// Let UE handle it
		return Device;
	}

	Device.DeviceIndex = DeviceIndex;

	Device.Joystick = SDL_JoystickOpen(DeviceIndex.value);
	if (Device.Joystick == nullptr)
	{
		return Device;
	}
	Device.InstanceId = FInstanceId(SDL_JoystickInstanceID(Device.Joystick));

	// DEBUG
	Device.Name = FString(ANSI_TO_TCHAR(SDL_JoystickName(Device.Joystick)));
	UE_LOG(JoystickPluginLog, Log, TEXT("--- %s"), *Device.Name);
	UE_LOG(JoystickPluginLog, Log, TEXT("--- Number of Axis %i"), SDL_JoystickNumAxes(Device.Joystick));
	UE_LOG(JoystickPluginLog, Log, TEXT("--- Number of Balls %i"), SDL_JoystickNumBalls(Device.Joystick));
	UE_LOG(JoystickPluginLog, Log, TEXT("--- Number of Buttons %i"), SDL_JoystickNumButtons(Device.Joystick));
	UE_LOG(JoystickPluginLog, Log, TEXT("--- Number of Hats %i"), SDL_JoystickNumHats(Device.Joystick));

	if (SDL_JoystickIsHaptic(Device.Joystick))
	{
		Device.Haptic = SDL_HapticOpenFromJoystick(Device.Joystick);
		if (Device.Haptic != nullptr)
		{
			UE_LOG(JoystickPluginLog, Log, TEXT("--- Rumble device detected"));

			if (SDL_HapticRumbleInit(Device.Haptic) != 0)
			{
				/*UE_LOG(JoystickPluginLog, Log, TEXT("--- testing Rumble device:"));
				if (SDL_HapticRumblePlay(OutDeviceInfo.Haptic, 0.5, 2000) != 0)
				{
				UE_LOG(JoystickPluginLog, Log, TEXT("--- play Rumble ...."));
				SDL_Delay(2000);
				}
				else
				{
				UE_LOG(JoystickPluginLog, Log, TEXT("--- not successful!"));
				SDL_HapticClose(OutDeviceInfo.Haptic);
				OutDeviceInfo.Haptic = nullptr;
				}*/
			}
		}
	}
	

	for (auto &ExistingDevice : Devices)
	{
		if (ExistingDevice.Value.Joystick == nullptr && ExistingDevice.Value.Name == Device.Name)
		{
			Device.DeviceId = ExistingDevice.Key;
			Devices[Device.DeviceId] = Device;

			DeviceMapping.Add(Device.InstanceId, Device.DeviceId);
			EventInterface->JoystickPluggedIn(Device);
			return Device;
		}
	}

	Device.DeviceId = FDeviceId(Devices.Num());
	Devices.Add(Device.DeviceId, Device);

	DeviceMapping.Add(Device.InstanceId, Device.DeviceId);
	EventInterface->JoystickPluggedIn(Device);
	return Device;
}

void FDeviceSDL::RemoveDevice(FDeviceId DeviceId)
{
	EventInterface->JoystickUnplugged(DeviceId);

	FDeviceInfoSDL &DeviceInfo = Devices[DeviceId];
	DeviceMapping.Remove(DeviceInfo.InstanceId);

	if (DeviceInfo.Haptic != nullptr)
	{
		SDL_HapticClose(DeviceInfo.Haptic);
		DeviceInfo.Haptic = nullptr;
	}

	if (DeviceInfo.Joystick != nullptr)
	{
		SDL_JoystickClose(DeviceInfo.Joystick);
		DeviceInfo.Joystick = nullptr;
	}
}

FString FDeviceSDL::DeviceGUIDtoString(FDeviceIndex DeviceIndex)
{
	char buffer[32];
	int8 sizeBuffer = sizeof(buffer);

	SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(DeviceIndex.value);
	SDL_JoystickGetGUIDString(guid, buffer, sizeBuffer);
	return ANSI_TO_TCHAR(buffer);
}

FGuid FDeviceSDL::DeviceGUIDtoGUID(FDeviceIndex DeviceIndex)
{
	FGuid result;
	SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(DeviceIndex.value);
	memcpy(&result, &guid, sizeof(FGuid));
	return result;
}

EJoystickPOVDirection SDL_hatValToDirection(int8 Value)
{
	switch (Value)
	{
	case SDL_HAT_CENTERED:  return EJoystickPOVDirection::DIRECTION_NONE;
	case SDL_HAT_UP:        return EJoystickPOVDirection::DIRECTION_UP;
	case SDL_HAT_RIGHTUP:   return EJoystickPOVDirection::DIRECTION_UP_RIGHT;
	case SDL_HAT_RIGHT:	    return EJoystickPOVDirection::DIRECTION_RIGHT;
	case SDL_HAT_RIGHTDOWN: return EJoystickPOVDirection::DIRECTION_DOWN_RIGHT;
	case SDL_HAT_DOWN:	    return EJoystickPOVDirection::DIRECTION_DOWN;
	case SDL_HAT_LEFTDOWN:  return EJoystickPOVDirection::DIRECTION_DOWN_LEFT;
	case SDL_HAT_LEFT:	    return EJoystickPOVDirection::DIRECTION_LEFT;
	case SDL_HAT_LEFTUP:    return EJoystickPOVDirection::DIRECTION_UP_LEFT;
	default:
		//UE_LOG(LogTemp, Warning, TEXT("Warning, POV unhandled case. %d"), (int32)value);
		return EJoystickPOVDirection::DIRECTION_NONE;
	}
}

FJoystickState FDeviceSDL::InitialDeviceState(FDeviceId DeviceId)
{
	FDeviceInfoSDL Device = Devices[DeviceId];
	FJoystickState State(DeviceId.value);

	if (Device.Joystick)
	{
		State.Axes.SetNumZeroed(SDL_JoystickNumAxes(Device.Joystick));
		State.Buttons.SetNumZeroed(SDL_JoystickNumButtons(Device.Joystick));
		State.Hats.SetNumZeroed(SDL_JoystickNumHats(Device.Joystick));
		State.Balls.SetNumZeroed(SDL_JoystickNumBalls(Device.Joystick));
	}
	
	//UE_LOG(JoystickPluginLog, Log, TEXT("DeviceSDL::getDeviceState() %s"), device.Name));

	return State;
}

void FDeviceSDL::Update()
{
	if (bOwnsSDL)
	{
		SDL_Event Event;
		while (SDL_PollEvent(&Event))
		{
			// The event watcher handles it
		}
	}
}

int FDeviceSDL::HandleSDLEvent(void* Userdata, SDL_Event* Event)
{
	FDeviceSDL& Self = *static_cast<FDeviceSDL*>(Userdata);

	switch (Event->type)
	{
	case SDL_JOYDEVICEADDED:
		Self.AddDevice(FDeviceIndex(Event->cdevice.which));
		break;
	/*case SDL_CONTROLLERDEVICEADDED:
	{
		if (Self.bIgnoreGameControllers)
		{
			// Since JOYSTICK is inited before GAMECONTROLLER (by GAMECONTROLLER), 
			// a controller can be added as a joystick before we can check that it is a controller.
			// Remove it again and let UE handle it.
			FDeviceIndex DeviceIndex = FDeviceIndex(Event->cdevice.which);
			for (auto &Device : Self.Devices)
			{
				if (Device.Value.DeviceIndex == DeviceIndex && Self.DeviceMapping.Contains(Device.Value.InstanceId))
				{
					Self.DeviceMapping.Remove(Device.Value.InstanceId);
					Self.EventInterface->JoystickUnplugged(Device.Value.DeviceId);
				}
			}
		}
		break;
	}*/
	case SDL_JOYDEVICEREMOVED:
	{
		FInstanceId InstanceId = FInstanceId(Event->cdevice.which);
		if (Self.DeviceMapping.Contains(InstanceId))
		{
			FDeviceId DeviceId = Self.DeviceMapping[InstanceId];
			Self.RemoveDevice(DeviceId);
		}
		break;
	}
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		if (Self.DeviceMapping.Contains(FInstanceId(Event->jbutton.which)))
		{
			FDeviceId DeviceId = Self.DeviceMapping[FInstanceId(Event->jbutton.which)];
			Self.EventInterface->JoystickButton(DeviceId, Event->jbutton.button, Event->jbutton.state == SDL_PRESSED);
		}
		break;
	case SDL_JOYAXISMOTION:
		if (Self.DeviceMapping.Contains(FInstanceId(Event->jaxis.which)))
		{
			FDeviceId DeviceId = Self.DeviceMapping[FInstanceId(Event->jaxis.which)];
			Self.EventInterface->JoystickAxis(DeviceId, Event->jaxis.axis, Event->jaxis.value / (Event->jaxis.value < 0 ? 32768.0f : 32767.0f));
		}
		break;
	case SDL_JOYHATMOTION:
		if (Self.DeviceMapping.Contains(FInstanceId(Event->jhat.which)))
		{
			FDeviceId DeviceId = Self.DeviceMapping[FInstanceId(Event->jhat.which)];
			Self.EventInterface->JoystickHat(DeviceId, Event->jhat.hat, SDL_hatValToDirection(Event->jhat.value));
		}
		break;
	case SDL_JOYBALLMOTION:
		if (Self.DeviceMapping.Contains(FInstanceId(Event->jball.which)))
		{
			FDeviceId DeviceId = Self.DeviceMapping[FInstanceId(Event->jball.which)];
			Self.EventInterface->JoystickBall(DeviceId, Event->jball.ball, FVector2D(Event->jball.xrel, Event->jball.yrel));
		}
		break;
	}

	return 0;
}

