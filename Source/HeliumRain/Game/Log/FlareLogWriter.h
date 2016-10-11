#pragma once
#include "../../Flare.h"


UENUM()
namespace EFlareLogTarget
{
	enum Type
	{
		Game,
		Combat,
	};
}

UENUM()
namespace EFlareLogEvent
{
	enum Type
	{
		// Game event
		GAME_LOADED,
		GAME_UNLOADED,
		DAY_SIMULATED,
		AI_CONSTRUCTION_STARTED,

		// Combat event
		SECTOR_ACTIVATED,
		SECTOR_DEACTIVATED,
		BOMB_DROPPED,
		BOMB_DESTROYED,
		SPACECRAFT_DAMAGED,
		SPACECRAFT_COMPONENT_DAMAGED
	};
}


UENUM()
namespace EFlareLogParam
{
	enum Type
	{
		String,
		Integer,
		Float,
		Vector3,
	};
}

struct FlareLogMessageParam
{
	EFlareLogParam::Type Type;
	FString StringValue;
	int64 IntValue;
	double FloatValue;
	FVector Vector3Value;
};

struct FlareLogMessage
{
	FDateTime Date;
	EFlareLogTarget::Type Target;
	EFlareLogEvent::Type Event;
	TArray<FlareLogMessageParam> Params;
};


//~~~~~ Multi Threading ~~~
class FFlareLogWriter : public FRunnable
{
	/** Singleton instance, can access the thread any time via static accessor, if it is active! */
	static  FFlareLogWriter* Runnable;

	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;

	/** Stop this thread? Uses Thread Safe Counter */
	FThreadSafeCounter StopTaskCounter;


	void InitLogFiles();

	void CloseLogFiles();

	IFileHandle* InitLogFile(FString BaseName);

	void WriteLine(IFileHandle* FileHandle, FString& Line);

	void WriteMessage(FlareLogMessage& Message);

	FString FormatMessage(FlareLogMessage& Message);

	FString FormatParam(FlareLogMessageParam* Param);

private:
	int32					PrimesFoundCount;
	FEvent*					NewMessageEvent;
	TQueue<FlareLogMessage>	MessageQueue;
	IFileHandle*			GameLogFile;
	IFileHandle*			CombatLogFile;
	FName					GameUUID;

public:


	//~~~ Thread Core Functions ~~~

	//Constructor / Destructor
	FFlareLogWriter(FName UUID);
	virtual ~FFlareLogWriter();

	void PushMessage(FlareLogMessage& Message);

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
	static FFlareLogWriter* InitWriter(FName UUID);
	static void PushWriterMessage(FlareLogMessage& Message);

	/** Shuts down the thread. Static so it can easily be called from outside the thread context */
	static void Shutdown();

};
