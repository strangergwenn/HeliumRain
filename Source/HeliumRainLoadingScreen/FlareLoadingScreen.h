#pragma once

#include <Modules/ModuleInterface.h>


class IFlareLoadingScreenModule : public IModuleInterface
{

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	virtual void StartInGameLoadingScreen() = 0;

};
