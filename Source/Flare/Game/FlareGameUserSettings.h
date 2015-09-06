#pragma once

#include "FlareGameUserSettings.generated.h"


UCLASS()
class FLARE_API UFlareGameUserSettings : public UGameUserSettings
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

	UPROPERTY(config)
	int32 ScreenPercentage;


};
