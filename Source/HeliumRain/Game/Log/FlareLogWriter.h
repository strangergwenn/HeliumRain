#pragma once
#include "../../Flare.h"

//~~~~~ Multi Threading ~~~
class FFlareLogWriter : public FRunnable
{
	/** Singleton instance, can access the thread any time via static accessor, if it is active! */
	static  FFlareLogWriter* Runnable;

	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;

	/** Stop this thread? Uses Thread Safe Counter */
	FThreadSafeCounter StopTaskCounter;

	//The actual finding of prime numbers
	int32 FindNextPrimeNumber();

private:
	int32				PrimesFoundCount;
	FEvent*				NewMessageEvent;
	TQueue<int32>			MessageQueue;

/*
	FlareLog::Combat->DamageDone(23);
	FlareLog::Combat->WeaponEmpty(23);


	FlareLog::Game->Loaded();
	FlareLog::Game->DamageDone(23);
	FlareLog::Game->Unloaded();

	FlareLog::Game->Trade();
	FlareLog::Game->Trade();
	FlareLog::Game->ReputationChange();*/



public:


	//~~~ Thread Core Functions ~~~

	//Constructor / Destructor
	FFlareLogWriter();
	virtual ~FFlareLogWriter();

	// Begin FRunnable interface.
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	// End FRunnable interface

	/** Makes sure this thread has stopped properly */
	void EnsureCompletion();



	//~~~ Starting and Stopping Thread ~~~



	/*
		Start the thread and the worker from static (easy access)!
		This code ensures only 1 thread will be able to run at a time.
		This function returns a handle to the newly started instance.
	*/
	static FFlareLogWriter* InitWriter();

	/** Shuts down the thread. Static so it can easily be called from outside the thread context */
	static void Shutdown();
};
