#pragma once

#include "CommonGOG.h"
#include "UniquePtr.h"
#include "Set.h"

// Interface needed to play-around lack of virtual inheritance in IGalaxy listeners
class IListenerGOG
{
public:

	virtual ~IListenerGOG() = default;

	FSetElementId ListenerID;
};

inline size_t GetTypeHash(const TUniquePtr<IListenerGOG>& inListener)
{
	return reinterpret_cast<size_t>(inListener.Get());
}