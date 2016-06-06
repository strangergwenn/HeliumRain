#pragma once

#include "FlareGameUserSettings.generated.h"


UCLASS()
class HELIUMRAIN_API UFlareGameUserSettings : public UGameUserSettings
{
public:

	GENERATED_UCLASS_BODY()

	/*----------------------------------------------------
		Public data
	----------------------------------------------------*/

public:

	virtual void SetToDefaults();

	virtual void ApplySettings(bool bCheckForCommandLineOverrides) override;

	void SetScreenPercentage(int32 NewScreenPercentage);

	/** Screen percentage */
	UPROPERTY(config)
	int32                                    ScreenPercentage;
		
	/** Whether to use the 3D cockpit */
	UPROPERTY(Config)
	bool                                     UseCockpit;
		
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
