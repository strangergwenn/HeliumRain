
#include "JoystickPlugin.h"
#include <Engine.h>

#if WITH_EDITOR
	#include "InputSettingsCustomization.h"
#endif

IMPLEMENT_MODULE(FJoystickPlugin, JoystickPlugin)

#define LOCTEXT_NAMESPACE "JoystickPlugin"

void FJoystickPlugin::StartupModule()
{
	IJoystickPlugin::StartupModule();
	JoystickDevice = MakeShareable(new ::FJoystickDevice());
	//return;

#if WITH_EDITOR
	// Replace parts of the input settings widget to make them wide enough to fit long joystick names
	FInputActionMappingCustomizationExtended::Register();
	FInputAxisMappingCustomizationExtended::Register();
#endif
}

#undef LOCTEXT_NAMESPACE
