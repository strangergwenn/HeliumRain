
#include "FlareLogWriter.h"
#include "../../Flare.h"
#include "FlareLogApi.h"
#include "../Save/FlareSaveWriter.h"

//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
FFlareLogWriter* FFlareLogWriter::Runnable = NULL;
//***********************************************************

static int ThreadIndex = 0;

FFlareLogWriter::FFlareLogWriter(FName UUID)
	: StopTaskCounter(0),
	  GameUUID(UUID)

{
	FString Name = TEXT("FFlareLogWriter-") + FString::FromInt(ThreadIndex);

	GameLogFile = NULL;
	CombatLogFile = NULL;

	Thread = FRunnableThread::Create(this, *Name, 0, TPri_BelowNormal); //windows default = 8mb for thread, could specify more
	ThreadIndex++;
}

FFlareLogWriter::~FFlareLogWriter()
{
	if (NewMessageEvent)
	{
		FPlatformProcess::ReturnSynchEventToPool(NewMessageEvent);
		NewMessageEvent = NULL;
	}

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
	// Open log files
	InitLogFiles();


	//While not told to stop this thread
	//		and not yet finished finding Prime Numbers
	while (StopTaskCounter.GetValue() == 0)
	{
		NewMessageEvent->Wait();

		FlareLogMessage Message;
		while (MessageQueue.Dequeue(Message))
		{
			WriteMessage(Message);
		}
	}

	CloseLogFiles();

	return 0;
}

//stop
void FFlareLogWriter::Stop()
{
	StopTaskCounter.Increment();
	if(NewMessageEvent)
	{
		NewMessageEvent->Trigger();
	}
}

FFlareLogWriter* FFlareLogWriter::InitWriter(FName UUID)
{
	//Create new instance of thread if it does not exist
	//		and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FFlareLogWriter(UUID);
		GameLog::GameLoaded();
		return Runnable;
	}

	return NULL;
}

void FFlareLogWriter::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

void FFlareLogWriter::Shutdown()
{
	if (Runnable)
	{
		GameLog::GameUnloaded();
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

void FFlareLogWriter::InitLogFiles()
{
	if(!GameLogFile)
	{
		GameLogFile = InitLogFile("Game");
	}

	if(!CombatLogFile)
	{
		CombatLogFile = InitLogFile("Combat");
	}
}

void FFlareLogWriter::CloseLogFiles()
{
	if (GameLogFile)
	{
		delete GameLogFile;
		GameLogFile = NULL;
	}

	if (CombatLogFile)
	{
		delete CombatLogFile;
		CombatLogFile = NULL;
	}
}

IFileHandle* FFlareLogWriter::InitLogFile(FString BaseName)
{
	FString FileName = FString::Printf(TEXT("%s/SaveGames/%s-%s.log"), *FPaths::GameSavedDir(), *BaseName, *GameUUID.ToString());

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	FLOGV("Init log file '%s'", *FileName);
	IFileHandle* FileHandle = PlatformFile.OpenWrite(*FileName, true);

	if (!FileHandle)
	{
		FLOGV("Fail to init log file '%s' for base name '%s'", *FileName, *BaseName);
	}

	return FileHandle;
}

void FFlareLogWriter::WriteLine(IFileHandle* FileHandle, FString& Line)
{
	FileHandle->Write((const uint8*)TCHAR_TO_ANSI(*Line), Line.Len());
}

void FFlareLogWriter::WriteMessage(FlareLogMessage& Message)
{
	FString MessageString = FormatMessage(Message);

	switch (Message.Target) {
	case EFlareLogTarget::Game:
		if (GameLogFile)
		{
			WriteLine(GameLogFile, MessageString);
		}
	break;
	case EFlareLogTarget::Combat:
		if (CombatLogFile)
		{
			WriteLine(CombatLogFile, MessageString);
		}
		break;

	default:
		break;
	}
}

FString FFlareLogWriter::FormatMessage(FlareLogMessage& Message)
{

	FString MessageString = FString::Printf(
				TEXT("%s %s"),
				*Message.Date.ToString(TEXT("%Y-%m-%dT%H:%M:%S.%s")),
				*UFlareSaveWriter::FormatEnum<EFlareLogEvent::Type>("EFlareLogEvent", Message.Event));

	for(int32 ParamIndex = 0; ParamIndex < Message.Params.Num(); ParamIndex++)
	{
		MessageString += "," + FormatParam(&Message.Params[ParamIndex]);
	}

	MessageString += "\n";
	return MessageString;
}

FString FFlareLogWriter::FormatParam(FlareLogMessageParam* Param)
{
	switch (Param->Type) {
	case EFlareLogParam::String:
		return "\""+Param->StringValue+"\"";
		break;
	case EFlareLogParam::Integer:
		return UFlareSaveWriter::FormatInt64(Param->IntValue);
		break;
	case EFlareLogParam::Float:
		return  FString::Printf(TEXT("%f"), Param->FloatValue);
		break;
	case EFlareLogParam::Vector3:
		return "("+UFlareSaveWriter::FormatVector(Param->Vector3Value)+")";
		break;
	default:
		FLOGV("Invalid log param type %d", (Param->Type + 0));
		break;
	}
	return "";
}

void FFlareLogWriter::PushMessage(FlareLogMessage& Message)
{
	Message.Date = FDateTime::UtcNow();
	MessageQueue.Enqueue(Message);
	NewMessageEvent->Trigger();
}

void FFlareLogWriter::PushWriterMessage(FlareLogMessage& Message)
{
	if (Runnable)
	{
		Runnable->PushMessage(Message);
	}
}
