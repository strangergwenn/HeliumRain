#pragma once

#include "ModuleInterface.h"


class IFlareLoadingScreenModule : public IModuleInterface
{

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void StartInGameLoadingScreen() = 0;

};
