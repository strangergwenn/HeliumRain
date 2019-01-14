#pragma once

#include "Types/IListenerGOG.h"

class FListenerManager
{
public:

	void FreeListener(FSetElementId InListenerID)
	{
		listenerRegistry.Remove(MoveTemp(InListenerID));
	}

	template<class Listener, typename... Args>
	TPair<FSetElementId, Listener*> CreateListener(Args&&... args)
	{
		auto listenerID = listenerRegistry.Add(MakeUnique<Listener>(Forward<Args>(args)...));
		const auto& listener = listenerRegistry[listenerID];
		listener->ListenerID = listenerID;

		return MakeTuple(listenerID, static_cast<Listener*>(listener.Get()));
	}

private:

	TSet<TUniquePtr<IListenerGOG>> listenerRegistry;
};