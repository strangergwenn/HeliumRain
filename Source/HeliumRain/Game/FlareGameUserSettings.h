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
	
	/** Whether to use the dark theme when no sector is active */
	UPROPERTY(Config)
	bool                                     UseDarkThemeForStrategy;
	
	/** Whether to use the dark theme when a sector is active */
	UPROPERTY(Config)
	bool                                     UseDarkThemeForNavigation;
	
	/** Whether to use the 3D cockpit */
	UPROPERTY(Config)
	bool                                     UseCockpit;
		
	/** Music volume */
	UPROPERTY(Config)
	int32                                    MusicVolume;

	/** Master volume */
	UPROPERTY(Config)
	int32                                    MasterVolume;
	
	/** Whether to use tessellation everywhere */
	UPROPERTY(Config)
	bool                                     UseTessellationOnShips;

};
