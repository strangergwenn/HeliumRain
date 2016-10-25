#pragma once

#include <IInputDevice.h>
#include "JoystickInterface.h"

struct FDeviceInfoSDL;
class FDeviceSDL;

class IJoystickEventInterface
{
public:
	virtual ~IJoystickEventInterface()
	{
	}

	virtual void JoystickPluggedIn(const FDeviceInfoSDL &Device) = 0;
	virtual void JoystickUnplugged(FDeviceId DeviceId) = 0;
	virtual void JoystickButton(FDeviceId DeviceId, int32 Button, bool Pressed) = 0;
	virtual void JoystickAxis(FDeviceId DeviceId, int32 Axis, float Value) = 0;
	virtual void JoystickHat(FDeviceId DeviceId, int32 Hat, EJoystickPOVDirection Value) = 0;
	virtual void JoystickBall(FDeviceId DeviceId, int32 Ball, FVector2D Delta) = 0;
};

class FJoystickDevice : public IInputDevice, public IJoystickEventInterface
{
public:
	FJoystickDevice();

	void Tick(float DeltaTime) override;
	void SendControllerEvents() override;
	void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;
	bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
	void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override;
	void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& Values) override;

	bool AddEventListener(UObject* Listener);
	void IgnoreGameControllers(bool bIgnore);

	virtual void JoystickPluggedIn(const FDeviceInfoSDL &Device) override;
	virtual void JoystickUnplugged(FDeviceId DeviceId) override;
	virtual void JoystickButton(FDeviceId DeviceId, int32 Button, bool Pressed) override;
	virtual void JoystickAxis(FDeviceId DeviceId, int32 Axis, float Value) override;
	virtual void JoystickHat(FDeviceId DeviceId, int32 Hat, EJoystickPOVDirection Value) override;
	virtual void JoystickBall(FDeviceId DeviceId, int32 Ball, FVector2D Delta) override;

	TMap<FDeviceId, FJoystickState> CurrentState;
	TMap<FDeviceId, FJoystickState> PreviousState;

	TMap<FDeviceId, FJoystickInfo> InputDevices;
private:
	void InitInputDevice(const FDeviceInfoSDL &Device);
	void EmitEvents(const FJoystickState& previous, const FJoystickState& current);

	TSharedPtr<FDeviceSDL> DeviceSDL;
	TArray<TWeakObjectPtr<UObject>> EventListeners;

	TMap<FDeviceId, TArray<FKey>> DeviceButtonKeys;
	TMap<FDeviceId, TArray<FKey>> DeviceAxisKeys;
	TMap<FDeviceId, TArray<FKey>> DeviceHatKeys[2];
	TMap<FDeviceId, TArray<FKey>> DeviceBallKeys[2];
};
