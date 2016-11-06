#pragma once

#include "FlareGameUserSettings.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareGameUserSettings : public UGameUserSettings
{
public:

	GENERATED_UCLASS_BODY()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void SetToDefaults();

	virtual void ApplySettings(bool bCheckForCommandLineOverrides) override;

	void SetScreenPercentage(int32 NewScreenPercentage);


	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

	/** Screen percentage */
	UPROPERTY(config)
	int32                                    ScreenPercentage;

	/** Whether to use motion blur */
	UPROPERTY(Config)
	bool                                     UseMotionBlur;

	/** Whether to use Temporal AA */
	UPROPERTY(Config)
	bool                                     UseTemporalAA;
		
	/** Whether to use the 3D cockpit */
	UPROPERTY(Config)
	bool                                     UseCockpit;
		
	/** Whether to use anti collision */
	UPROPERTY(Config)
	bool                                     UseAnticollision;

	/** Whether to pause the game in menus */
	UPROPERTY(Config)
	bool                                     PauseGameInMenus;
		
	/** Music volume */
	UPROPERTY(Config)
	int32                                    MusicVolume;

	/** Master volume */
	UPROPERTY(Config)
	int32                                    MasterVolume;

};
