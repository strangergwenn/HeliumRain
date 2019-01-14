#pragma once

#include "OnlineSubsystem.h"
#include "OnlineSubsystemGOG.h"

class FOnlineFactoryGOG : public IOnlineFactory
{
public:

	FOnlineFactoryGOG();

	virtual ~FOnlineFactoryGOG();

	virtual IOnlineSubsystemPtr CreateSubsystem(FName InInstanceName) override;

private:

	void DestroyOnlineSubsystemGOG();

	FOnlineSubsystemGOGPtr onlineSubsystemGOG;
};