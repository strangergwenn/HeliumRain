#pragma once

#include "Engine.h"
#include "JoystickInterface.generated.h"

struct FDeviceIndex
{
	int32 value = -1;
	explicit FDeviceIndex(int32 v) : value(v) {}
	bool operator==(const FDeviceIndex other) const { return value == other.value; };
};

struct FInstanceId
{
	int32 value = -1;
	explicit FInstanceId(int32 v) : value(v) {}
	bool operator==(FInstanceId other) const { return value == other.value; };
};
FORCEINLINE uint32 GetTypeHash(FInstanceId instanceId)
{
	return GetTypeHash(instanceId.value);
}

struct FDeviceId
{
	int32 value = -1;
	explicit FDeviceId(int32 v) : value(v) {}
	bool operator==(FDeviceId other) const { return value == other.value; };
};
FORCEINLINE uint32 GetTypeHash(FDeviceId deviceId)
{
	return GetTypeHash(deviceId.value);
}


UENUM(BlueprintType)
enum class EInputType : uint8
{
	INPUTTYPE_UNKNOWN,
	INPUTTYPE_JOYSTICK,
	INPUTTYPE_GAMECONTROLLER,	
};


UENUM(BlueprintType)
enum class EJoystickPOVDirection : uint8
{
	DIRECTION_NONE,
	DIRECTION_UP,
	DIRECTION_UP_RIGHT,
	DIRECTION_RIGHT,
	DIRECTION_DOWN_RIGHT,
	DIRECTION_DOWN,
	DIRECTION_DOWN_LEFT,
	DIRECTION_LEFT,
	DIRECTION_UP_LEFT,
};

FVector2D POVAxis(EJoystickPOVDirection povValue);

USTRUCT(BlueprintType)
struct FJoystickState
{
	GENERATED_USTRUCT_BODY()

        explicit FJoystickState(int32 DeviceIdParam = -1)
        : DeviceId(DeviceIdParam)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = JoystickState)
	int32 DeviceId;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = JoystickState)
	TArray<float> Axes;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = JoystickState)
	TArray<bool> Buttons;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = JoystickState)
	TArray<EJoystickPOVDirection> Hats;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = JoystickState)
	TArray<FVector2D> Balls;
};

USTRUCT(BlueprintType)
struct FJoystickInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = JoystickInfo)
	int32 Player = -1;
	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = JoystickInfo)
	int32 DeviceId = -1;
	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = JoystickInfo)
	bool IsRumbleDevice = false;

	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = JoystickInfo)
	FGuid ProductId;
	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = JoystickInfo)
	FString ProductName;
	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = JoystickInfo)
	FString DeviceName;
	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = JoystickInfo)
	bool Connected = false;
	UPROPERTY(EditAnywhere, BlueprintReadonly, Category = JoystickInfo)
	TArray<EInputType> InputType;
};

UINTERFACE(MinimalAPI)
class UJoystickInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class IJoystickInterface
{
	GENERATED_IINTERFACE_BODY()

public:

	//Define blueprint events
	UFUNCTION(BlueprintImplementableEvent, Category = "Joystick")
	void JoystickButtonPressed(int32 Button, FJoystickState state);

	UFUNCTION(BlueprintImplementableEvent, Category = "Joystick")
	void JoystickButtonReleased(int32 Button, FJoystickState state);

	UFUNCTION(BlueprintImplementableEvent, Category = "Joystick")
	void JoystickAxisChanged(int32 Axis, float value, float valuePrev, FJoystickState state, FJoystickState prev);

	UFUNCTION(BlueprintImplementableEvent, Category = "Joystick")
	void JoystickHatChanged(int32 Hat, EJoystickPOVDirection Value, FJoystickState state);

	UFUNCTION(BlueprintImplementableEvent, Category = "Joystick")
	void JoystickBallMoved(int32 Ball, FVector2D Delta, FJoystickState State);

	UFUNCTION(BlueprintImplementableEvent, Category = "Joystick")
	void JoystickPluggedIn(int32 DeviceId);

	UFUNCTION(BlueprintImplementableEvent, Category = "Joystick")
	void JoystickUnplugged(int32 DeviceId);

	virtual FString ToString();
};
