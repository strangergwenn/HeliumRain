#include "../../Flare.h"
#include "FlareLogApi.h"
#include "FlareLogWriter.h"

#include "../FlareCompany.h"
#include "../FlareSimulatedSector.h"
#include "../../Spacecrafts/FlareSimulatedSpacecraft.h"

// Game log api

void GameLog::GameLoaded()
{
	FlareLogMessage Message;
	Message.Target = EFlareLogTarget::Game;
	Message.Event = EFlareLogEvent::GAME_LOADED;
	FFlareLogWriter::PushWriterMessage(Message);
}

void GameLog::GameUnloaded()
{
	FlareLogMessage Message;
	Message.Target = EFlareLogTarget::Game;
	Message.Event = EFlareLogEvent::GAME_UNLOADED;
	FFlareLogWriter::PushWriterMessage(Message);
}


void GameLog::DaySimulated(int64 NewDate)
{
	FlareLogMessage Message;
	Message.Target = EFlareLogTarget::Game;
	Message.Event = EFlareLogEvent::DAY_SIMULATED;

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::Integer;
		Param.IntValue = NewDate;
		Message.Params.Add(Param);
	}

	FFlareLogWriter::PushWriterMessage(Message);
}

void GameLog::AIConstructionStart(UFlareCompany* Company,
								UFlareSimulatedSector* ConstructionProjectSector,
								FFlareSpacecraftDescription* ConstructionProjectStationDescription,
								UFlareSimulatedSpacecraft* ConstructionProjectStation)
{
	FlareLogMessage Message;
	Message.Target = EFlareLogTarget::Game;
	Message.Event = EFlareLogEvent::AI_CONSTRUCTION_STARTED;

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = Company->GetCompanyName().ToString();
		Message.Params.Add(Param);
	}
	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = ConstructionProjectSector->GetIdentifier().ToString();
		Message.Params.Add(Param);
	}
	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = ConstructionProjectStationDescription->Identifier.ToString();
		Message.Params.Add(Param);
	}
	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = (ConstructionProjectStation ? ConstructionProjectStation->GetImmatriculation().ToString() : "");
		Message.Params.Add(Param);
	}

	FFlareLogWriter::PushWriterMessage(Message);
}
