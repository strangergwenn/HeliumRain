/*
*
* Copyright (C) <2014> samiljan <Sam Persson>, tsky <thomas.kollakowksy@w-hs.de>
* All rights reserved.
*
* This software may be modified and distributed under the terms
* of the BSD license.  See the LICENSE file for details.
*/

#pragma once

#include "JoystickInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(JoystickPluginLog, Log, All);

class IJoystickEventInterface;
struct _SDL_Joystick;
typedef struct _SDL_Joystick SDL_Joystick;
struct _SDL_Haptic;
typedef struct _SDL_Haptic SDL_Haptic;
struct _SDL_GameController;
typedef struct _SDL_GameController SDL_GameController;

union SDL_Event;

struct FDeviceInfoSDL
{
	FDeviceInfoSDL() {}

	FDeviceIndex DeviceIndex {0};
	FDeviceId DeviceId {0};
	FInstanceId InstanceId {0};

	FString Name = "unknown";

	SDL_Haptic* Haptic = nullptr;
	SDL_Joystick* Joystick = nullptr;
};

class FDeviceSDL
{
public:
	FJoystickState InitialDeviceState(FDeviceId DeviceId);

	static FString DeviceGUIDtoString(FDeviceIndex DeviceIndex);
	static FGuid DeviceGUIDtoGUID(FDeviceIndex DeviceIndex);

	FDeviceInfoSDL * GetDevice(FDeviceId DeviceId);
	
	void IgnoreGameControllers(bool bIgnore);

	void Update();

	FDeviceSDL(IJoystickEventInterface * EventInterface);
	void Init();
	virtual ~FDeviceSDL();

private:
	FDeviceInfoSDL AddDevice(FDeviceIndex DeviceIndex);
	void RemoveDevice(FDeviceId DeviceId);

	TMap<FDeviceId, FDeviceInfoSDL> Devices;	
	TMap<FInstanceId, FDeviceId> DeviceMapping;

	IJoystickEventInterface* EventInterface;

	bool bOwnsSDL;

	bool bIgnoreGameControllers = true;

	static int HandleSDLEvent(void* Userdata, SDL_Event* Event);
};
