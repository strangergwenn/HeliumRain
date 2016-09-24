#include "../../Flare.h"
#include "FlareLogWriter.h"


//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
FFlareLogWriter* FFlareLogWriter::Runnable = NULL;
//***********************************************************

static int ThreadIndex = 0;

FFlareLogWriter::FFlareLogWriter()
	: StopTaskCounter(0)

{
	FString Name = TEXT("FFlareLogWriter-") + FString::FromInt(ThreadIndex);

	Thread = FRunnableThread::Create(this, *Name, 0, TPri_BelowNormal); //windows default = 8mb for thread, could specify more
	ThreadIndex++;
	FLOGV("Create Thread: %p %s", Thread, *Name);
}

FFlareLogWriter::~FFlareLogWriter()
{
	if (NewMessageEvent)
	{
		FPlatformProcess::ReturnSynchEventToPool(NewMessageEvent);
	}

	FLOGV("Delete Thread: %p", Thread);
	delete Thread;
	Thread = NULL;
}

//Init
bool FFlareLogWriter::Init()
{
	//Init the Data
	NewMessageEvent = FPlatformProcess::GetSynchEventFromPool(false);

	return true;
}

//Run
uint32 FFlareLogWriter::Run()
{

	FLOG("Log writer created");

	//While not told to stop this thread
	//		and not yet finished finding Prime Numbers
	while (StopTaskCounter.GetValue() == 0)
	{
		NewMessageEvent->Wait();
		FLOG("Wake log writer !");
	}

	FLOG("Log writer deleted");

	return 0;
}

//stop
void FFlareLogWriter::Stop()
{
	StopTaskCounter.Increment();
	NewMessageEvent->Trigger();
}

FFlareLogWriter* FFlareLogWriter::InitWriter()
{
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FFlareLogWriter();
	}
	return Runnable;
	return NULL;
}

void FFlareLogWriter::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

void FFlareLogWriter::Shutdown()
{
	FLOG("Sleep");
	FPlatformProcess::Sleep(1.f);
	FLOG("Sleep done");
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
	FPlatformProcess::Sleep(1.f);
}
