
#include "JoystickDevice.h"
#include <Engine.h>


#include "DeviceSDL.h"

#include <SlateBasics.h>
#include <Internationalization/Text.h>

#define LOCTEXT_NAMESPACE "JoystickPlugin"

//Init and Runtime

FJoystickDevice::FJoystickDevice()
{
	UE_LOG(JoystickPluginLog, Log, TEXT("FJoystickPlugin::StartupModule() creating Device SDL"));

	DeviceSDL = MakeShareable(new FDeviceSDL(this));
	DeviceSDL->Init();
}

void FJoystickDevice::InitInputDevice(const FDeviceInfoSDL &Device)
{
	FDeviceId DeviceId = Device.DeviceId;
	FJoystickInfo DeviceInfo;

	DeviceInfo.Connected = true;
	DeviceInfo.DeviceId = DeviceId.value;
	DeviceInfo.Player = 0;

	DeviceInfo.ProductId = FDeviceSDL::DeviceGUIDtoGUID(Device.DeviceIndex);
	DeviceInfo.ProductName = Device.Name;
	DeviceInfo.ProductName = DeviceInfo.ProductName.Replace(TEXT("."), TEXT(""));
	DeviceInfo.DeviceName = DeviceInfo.ProductName.Replace(TEXT(" "), TEXT(""));
	DeviceInfo.DeviceName = DeviceInfo.DeviceName.Replace(TEXT("."), TEXT(""));

	UE_LOG(JoystickPluginLog, Log, TEXT("add device %s %i"), *DeviceInfo.DeviceName, DeviceId.value);
	InputDevices.Emplace(DeviceId, DeviceInfo);

	FJoystickState InitialState = DeviceSDL->InitialDeviceState(DeviceId);
	PreviousState.Emplace(DeviceId, InitialState);
	CurrentState.Emplace(DeviceId, InitialState);

	// create FKeyDetails for axis
	DeviceAxisKeys.Emplace(DeviceId);
	for (int iAxis = 0; iAxis < InitialState.Axes.Num(); iAxis++)
	{
		FString strName = FString::Printf(TEXT("Joystick_%s_%d_Axis%d"), *DeviceInfo.DeviceName.Left(15), DeviceId.value + 1, iAxis);
		UE_LOG(JoystickPluginLog, Log, TEXT("add %s %i"), *strName, DeviceId.value);
		DeviceAxisKeys[DeviceId].Add(FKey(FName(*strName)));

		if (!EKeys::GetKeyDetails(DeviceAxisKeys[DeviceId][iAxis]).IsValid())
		{
			FText textValue = FText::FromString(strName);
			EKeys::AddKey(FKeyDetails(DeviceAxisKeys[DeviceId][iAxis], textValue, FKeyDetails::GamepadKey | FKeyDetails::FloatAxis));
		}
	}

	// create FKeyDetails for buttons
	DeviceButtonKeys.Emplace(DeviceId);
	for (int iButton = 0; iButton < InitialState.Buttons.Num(); iButton++)
	{
		FString strName = FString::Printf(TEXT("Joystick_%s_%d_Button%d"), *DeviceInfo.DeviceName.Left(15), DeviceId.value + 1, iButton);
		UE_LOG(JoystickPluginLog, Log, TEXT("add %s %i"), *strName, DeviceId.value);
		DeviceButtonKeys[DeviceId].Add(FKey(FName(*strName)));

		if (!EKeys::GetKeyDetails(DeviceButtonKeys[DeviceId][iButton]).IsValid())
		{
			FText textValue = FText::FromString(strName);
			EKeys::AddKey(FKeyDetails(DeviceButtonKeys[DeviceId][iButton], textValue, FKeyDetails::GamepadKey));
		}
	}

	FString _2DaxisNames[] = { TEXT("X"), TEXT("Y") };

	// create FKeyDetails for hats
	for (int iAxis = 0; iAxis < 2; iAxis++)
	{
		DeviceHatKeys[iAxis].Emplace(DeviceId);
		for (int iHat = 0; iHat < InitialState.Hats.Num(); iHat++)
		{
			FString strName = FString::Printf(TEXT("Joystick_%s_%d_Hat%d_%s"), *DeviceInfo.DeviceName.Left(15), DeviceId.value + 1, iHat, *_2DaxisNames[iAxis]);
			UE_LOG(JoystickPluginLog, Log, TEXT("add %s %i"), *strName, DeviceId.value);
			FKey key{ *strName };
			DeviceHatKeys[iAxis][DeviceId].Add(key);

			if (!EKeys::GetKeyDetails(key).IsValid())
			{
				FText textValue = FText::FromString(strName);
				EKeys::AddKey(FKeyDetails(key, textValue, FKeyDetails::GamepadKey | FKeyDetails::FloatAxis));
			}
		}
	}

	// create FKeyDetails for balls
	for (int iAxis = 0; iAxis < 2; iAxis++)
	{
		DeviceBallKeys[iAxis].Emplace(DeviceId);
		for (int iBall = 0; iBall < InitialState.Balls.Num(); iBall++)
		{
			FString strName = FString::Printf(TEXT("Joystick_%s_%d_Ball%d_%s"), *DeviceInfo.DeviceName.Left(15), DeviceId.value + 1, iBall, *_2DaxisNames[iAxis]);
			UE_LOG(JoystickPluginLog, Log, TEXT("add %s %i"), *strName, DeviceId.value);
			FKey key{ *strName };
			DeviceBallKeys[iAxis][DeviceId].Add(key);

			if (!EKeys::GetKeyDetails(key).IsValid())
			{
				FText textValue = FText::FromString(strName);
				EKeys::AddKey(FKeyDetails(key, textValue, FKeyDetails::GamepadKey | FKeyDetails::FloatAxis));
			}
		}
	}
}

//Public API Implementation

void FJoystickDevice::JoystickPluggedIn(const FDeviceInfoSDL &DeviceInfoSDL)
{
	UE_LOG(JoystickPluginLog, Log, TEXT("FJoystickPlugin::JoystickPluggedIn() %i"), DeviceInfoSDL.DeviceId.value);

	InitInputDevice(DeviceInfoSDL);
	for (auto & listener : EventListeners)
	{
		UObject * o = listener.Get();
		if (o != nullptr)
		{
			IJoystickInterface::Execute_JoystickPluggedIn(o, DeviceInfoSDL.DeviceId.value);
		}
	}
}

void FJoystickDevice::JoystickUnplugged(FDeviceId DeviceId)
{
	InputDevices[DeviceId].Connected = false;

	UE_LOG(JoystickPluginLog, Log, TEXT("Joystick %d disconnected"), DeviceId.value);

	for (auto & listener : EventListeners)
	{
		UObject * o = listener.Get();
		if (o != nullptr)
		{
			IJoystickInterface::Execute_JoystickUnplugged(o, DeviceId.value);
		}
	}
}

bool EmitKeyUpEventForKey(FKey Key, int32 User, bool Repeat)
{
	FKeyEvent KeyEvent(Key, FSlateApplication::Get().GetModifierKeys(), User, Repeat, 0, 0);
	return FSlateApplication::Get().ProcessKeyUpEvent(KeyEvent);
}

bool EmitKeyDownEventForKey(FKey Key, int32 User, bool Repeat)
{
	FKeyEvent KeyEvent(Key, FSlateApplication::Get().GetModifierKeys(), User, Repeat, 0, 0);
	return FSlateApplication::Get().ProcessKeyDownEvent(KeyEvent);
}

void FJoystickDevice::JoystickButton(FDeviceId DeviceId, int32 Button, bool Pressed)
{
	CurrentState[DeviceId].Buttons[Button] = Pressed;
	if (Pressed)
		EmitKeyDownEventForKey(DeviceButtonKeys[DeviceId][Button], InputDevices[DeviceId].Player, false);
	else
		EmitKeyUpEventForKey(DeviceButtonKeys[DeviceId][Button], InputDevices[DeviceId].Player, false);

	for (auto & listener : EventListeners)
	{
		UObject * o = listener.Get();
		if (o != nullptr)
		{
			if (Pressed)
				IJoystickInterface::Execute_JoystickButtonPressed(o, Button, CurrentState[DeviceId]);
			else
				IJoystickInterface::Execute_JoystickButtonReleased(o, Button, CurrentState[DeviceId]);
		}
	}
}

bool EmitAnalogInputEventForKey(FKey Key, float Value, int32 User, bool Repeat)
{
	FAnalogInputEvent AnalogInputEvent(Key, FSlateApplication::Get().GetModifierKeys(), User, Repeat, 0, 0, Value);
	return FSlateApplication::Get().ProcessAnalogInputEvent(AnalogInputEvent);
}

void FJoystickDevice::JoystickAxis(FDeviceId DeviceId, int32 Axis, float Value)
{
	CurrentState[DeviceId].Axes[Axis] = Value;
	EmitAnalogInputEventForKey(DeviceAxisKeys[DeviceId][Axis], Value, InputDevices[DeviceId].Player, 0);

	for (auto & listener : EventListeners)
	{
		UObject * o = listener.Get();
		if (o != nullptr)
		{
			IJoystickInterface::Execute_JoystickAxisChanged(o, Axis, CurrentState[DeviceId].Axes[Axis], PreviousState[DeviceId].Axes[Axis], CurrentState[DeviceId], PreviousState[DeviceId]);
		}
	}
}

void FJoystickDevice::JoystickHat(FDeviceId DeviceId, int32 Hat, EJoystickPOVDirection Value)
{
	CurrentState[DeviceId].Hats[Hat] = Value;

	FVector2D povAxis = POVAxis(Value);
	EmitAnalogInputEventForKey(DeviceHatKeys[0][DeviceId][Hat], povAxis.X, InputDevices[DeviceId].Player, 0);
	EmitAnalogInputEventForKey(DeviceHatKeys[1][DeviceId][Hat], povAxis.Y, InputDevices[DeviceId].Player, 0);

	for (auto & listener : EventListeners)
	{
		UObject * o = listener.Get();
		if (o != nullptr)
		{
			IJoystickInterface::Execute_JoystickHatChanged(o, Hat, Value, CurrentState[DeviceId]);
		}
	}
}

bool EmitPointerEventForKey(int32 PointerIndex, const FVector2D &Value)
{
	FPointerEvent pointerEvent(PointerIndex, FVector2D::ZeroVector, FVector2D::ZeroVector, Value, TSet<FKey>(), FSlateApplication::Get().GetModifierKeys());
	return FSlateApplication::Get().ProcessMouseMoveEvent(pointerEvent);
}

void FJoystickDevice::JoystickBall(FDeviceId DeviceId, int32 Ball, FVector2D Delta)
{
	CurrentState[DeviceId].Balls[Ball] = Delta;

	//EmitPointerEventForKey(ball, FVector2D(dx, dy)); Maybe try something like this instead?

	EmitAnalogInputEventForKey(DeviceBallKeys[0][DeviceId][Ball], Delta.X, InputDevices[DeviceId].Player, 0);
	EmitAnalogInputEventForKey(DeviceBallKeys[1][DeviceId][Ball], Delta.Y, InputDevices[DeviceId].Player, 0);

	for (auto & listener : EventListeners)
	{
		UObject * o = listener.Get();
		if (o != nullptr)
		{
			IJoystickInterface::Execute_JoystickBallMoved(o, Ball, Delta, CurrentState[DeviceId]);
		}
	}
}

void FJoystickDevice::EmitEvents(const FJoystickState &previous, const FJoystickState &current)
{
	for (int iButton = 0; iButton < current.Buttons.Num(); iButton++)
	{
		if (previous.Buttons[iButton] != current.Buttons[iButton])
		{
			JoystickButton(FDeviceId(current.DeviceId), iButton, current.Buttons[iButton]);
		}
	}
	for (int iAxis = 0; iAxis < current.Axes.Num(); iAxis++)
	{
		if (previous.Axes[iAxis] != current.Axes[iAxis])
		{
			JoystickAxis(FDeviceId(current.DeviceId), iAxis, current.Axes[iAxis]);
		}
	}
	for (int iHat = 0; iHat < current.Hats.Num(); iHat++)
	{
		if (previous.Hats[iHat] != current.Hats[iHat])
		{
			JoystickHat(FDeviceId(current.DeviceId), iHat, current.Hats[iHat]);
		}
	}
	for (int iBall = 0; iBall < current.Balls.Num(); iBall++)
	{
		if (current.Balls[iBall] != FVector2D::ZeroVector)
		{
			JoystickBall(FDeviceId(current.DeviceId), iBall, current.Balls[iBall]);
		}
	}
}


// IInputDevice implementation

void FJoystickDevice::Tick(float DeltaTime)
{

}

void FJoystickDevice::SendControllerEvents()
{
	for (auto & Device : InputDevices)
	{
		FDeviceId DeviceId = Device.Key;
		if (InputDevices.Contains(DeviceId)) 
		{
			if (InputDevices[DeviceId].Connected) 
			{
				PreviousState[DeviceId] = FJoystickState(CurrentState[DeviceId]);

				for (int iBall = 0; iBall < CurrentState[DeviceId].Balls.Num(); iBall++)
				{
					CurrentState[DeviceId].Balls[iBall] = FVector2D::ZeroVector;
				}
			}
		}
	}

	DeviceSDL->Update();

	// Clean up weak references
	for (int i = 0; i < EventListeners.Num(); i++)
	{
		if (!EventListeners[i].IsValid())
		{
			EventListeners.RemoveAt(i);
			i--;
		}
	}
}

void FJoystickDevice::SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
}

bool FJoystickDevice::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	return false;
}

void FJoystickDevice::SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value)
{
	//FControllerState& ControllerState = ControllerStates[ControllerId];
	//if (ControllerState.bIsConnected)
	//{
	//	switch (ChannelType)
	//	{
	//	case FF_CHANNEL_LEFT_LARGE:
	//		ControllerState.ForceFeedback.LeftLarge = Value;
	//		break;
	//	case FF_CHANNEL_LEFT_SMALL:
	//		ControllerState.ForceFeedback.LeftSmall = Value;
	//		break;
	//	case FF_CHANNEL_RIGHT_LARGE:
	//		ControllerState.ForceFeedback.RightLarge = Value;
	//		break;
	//	case FF_CHANNEL_RIGHT_SMALL:
	//		ControllerState.ForceFeedback.RightSmall = Value;
	//		break;
	//	}
	//}
}

void FJoystickDevice::SetChannelValues(int32 ControllerId, const FForceFeedbackValues& Values)
{
	//FControllerState& ControllerState = ControllerStates[ControllerId];
	//if (ControllerState.bIsConnected)
	//{
	//	ControllerState.ForceFeedback = Values;
	//}
}

bool FJoystickDevice::AddEventListener(UObject* Listener)
{
	if (Listener != nullptr && Listener->GetClass()->ImplementsInterface(UJoystickInterface::StaticClass()))
	{
		EventListeners.Add(TWeakObjectPtr<>(Listener));
		return true;
	}
	return false;
}

void FJoystickDevice::IgnoreGameControllers(bool bIgnore)
{
	DeviceSDL->IgnoreGameControllers(bIgnore);
}
#undef LOCTEXT_NAMESPACE
