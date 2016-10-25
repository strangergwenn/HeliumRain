#include "JoystickInterface.h"

#include "JoystickFunctions.generated.h"

UCLASS(BlueprintType)
class UJoystickFunctions : public UObject
{
	GENERATED_UCLASS_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Input|Joystick input")
	static FVector2D POVAxis(EJoystickPOVDirection Direction);

	UFUNCTION(BlueprintPure, Category = "Input|Joystick input")
    static FJoystickInfo GetJoystick(int32 DeviceId);

	UFUNCTION(BlueprintPure, Category = "Input|Joystick input")
	static FJoystickState GetJoystickState(int32 DeviceId);

	UFUNCTION(BlueprintPure, Category = "Input|Joystick input")
	static FJoystickState GetPreviousJoystickState(int32 DeviceId);

	UFUNCTION(BlueprintPure, Category = "Input|Joystick input")
	static int32 JoystickCount();

	UFUNCTION(BlueprintCallable, Category = "Input|Joystick input")
	static void RegisterForJoystickEvents(UObject* Listener);

	UFUNCTION(BlueprintCallable, Category = "Input|Joystick input")
	static void MapJoystickDeviceToPlayer(int32 DeviceId, int32 Player);

	UFUNCTION(BlueprintCallable, Category = "Input|Joystick input")
	static void IgnoreGameControllers(bool bIgnore);
};
