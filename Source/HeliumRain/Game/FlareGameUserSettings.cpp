
#include "FlareGameUserSettings.h"
#include "../Flare.h"
#include "FlareGame.h"
#include "../Player/FlarePlayerController.h"

#include "EngineUtils.h"
#include "Engine.h"


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
	MonitorNumber = 0;
	VerticalFOV = 60;
	ScreenPercentage = 100;
	UseTemporalAA = true;
	UseMotionBlur = true;
	Gamma = 2.2f;

	// Gameplay
	InvertY = false;
	UseCockpit = true;
	UseAnticollision = false;
	PauseGameInMenus = false;
	MaxShipsInSector = 30;

	// Input
	GamepadProfileLayout = EFlareGamepadLayout::GL_Default;
	ForwardOnlyThrust = false;
	RotationDeadZone = 0.0f;
	TranslationDeadZone = 0.0f;
	RollDeadZone = 0.0f;
	InputSensitivity = 1.0f;

	// Sound
	MusicVolume = 10;
	MasterVolume = 10;
}

void UFlareGameUserSettings::EnsureConsistency()
{
	FLOG("UFlareGameUserSettings::EnsureConsistency");

	if (VerticalFOV < 60)
	{
		VerticalFOV = 60;
	}
	if (Gamma < KINDA_SMALL_NUMBER)
	{
		Gamma = 2.2f;
	}
	if (InputSensitivity < KINDA_SMALL_NUMBER)
	{
		InputSensitivity = 1.0f;
	}
	if (TurnWithLeftStick)
	{
		GamepadProfileLayout = EFlareGamepadLayout::GL_TurnWithLeftStick;
	}

	ApplySettings(false);
}

void UFlareGameUserSettings::ApplySettings(bool bCheckForCommandLineOverrides)
{
	FLOG("UFlareGameUserSettings::ApplySettings");

	Super::ApplySettings(bCheckForCommandLineOverrides);

	SetUseTemporalAA(UseTemporalAA);

	SetScreenPercentage(ScreenPercentage);

	// MonitorNumber is a human value (1-3 etc), with 0 defined as "default system"
	if (MonitorNumber > 0)
	{
		MoveToMonitor(MonitorNumber - 1);
	}
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

int32 UFlareGameUserSettings::GetMonitorCount() const
{
	if (GEngine && GEngine->GameViewport)
	{
		FDisplayMetrics Display;
		FSlateApplication::Get().GetDisplayMetrics(Display);
		return Display.MonitorInfo.Num();
	}

	return 0;
}

void UFlareGameUserSettings::MoveToMonitor(int32 NewMonitorIndex)
{
	if (GEngine && GEngine->GameViewport)
	{
		FDisplayMetrics Display;
		FSlateApplication::Get().GetDisplayMetrics(Display);

		// Found it
		if (NewMonitorIndex >= 0 && NewMonitorIndex < Display.MonitorInfo.Num())
		{
			FLOGV("UFlareGameUserSettings::MoveToMonitor : moving to monitor %d", NewMonitorIndex);
			
			// Compute position
			int32 XPosition = 0;
			for (int32 i = 0; i < NewMonitorIndex; i++)
			{
				XPosition += Display.MonitorInfo[i].NativeWidth;
			}
			XPosition -= Display.MonitorInfo[NewMonitorIndex].NativeWidth;

			// Move
			GEngine->GameViewport->GetWindow()->MoveWindowTo(FVector2D(XPosition, 0));
		}

		// Nope
		else
		{
			FLOGV("UFlareGameUserSettings::MoveToMonitor : couldn't find monitor %d", NewMonitorIndex);
		}
	}
}
