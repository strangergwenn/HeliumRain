
#include "FlareGameUserSettings.h"
#include "../Flare.h"
#include "FlareGame.h"
#include "../Player/FlarePlayerController.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareGameUserSettings::UFlareGameUserSettings(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}

void UFlareGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();

	// Graphics
	VerticalFOV = 60;
	ScreenPercentage = 100;
	UseTemporalAA = true;
	UseMotionBlur = true;

	// Gameplay
	InvertY = false;
	UseCockpit = true;
	UseAnticollision = false;
	PauseGameInMenus = false;
	MaxShipsInSector = 30;

	// Input
	TurnWithLeftStick = false;
	ForwardOnlyThrust = false;
	RotationDeadZone = 0.0f;
	TranslationDeadZone = 0.0f;
	RollDeadZone = 0.0f;

	// Sound
	MusicVolume = 10;
	MasterVolume = 10;
}

void UFlareGameUserSettings::ApplySettings(bool bCheckForCommandLineOverrides)
{
	FLOG("UFlareGameUserSettings::ApplySettings");

	Super::ApplySettings(bCheckForCommandLineOverrides);

	SetUseTemporalAA(UseTemporalAA);

	SetScreenPercentage(ScreenPercentage);
}

void UFlareGameUserSettings::SetUseTemporalAA(bool NewSetting)
{
	//FLOGV("UFlareGameUserSettings::SetUseTemporalAA %d", NewSetting);
	UseTemporalAA = NewSetting;

	auto TemporalAACVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DefaultFeature.AntiAliasing"));
	TemporalAACVar->Set(UseTemporalAA ? AAM_TemporalAA : AAM_FXAA, ECVF_SetByConsole);
}

void UFlareGameUserSettings::SetScreenPercentage(int32 NewScreenPercentage)
{
	//FLOGV("UFlareGameUserSettings::SetScreenPercentage %d", NewScreenPercentage);
	ScreenPercentage = NewScreenPercentage;

	auto ScreenPercentageCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
	ScreenPercentageCVar->Set(ScreenPercentage, ECVF_SetByConsole);
}
