
#include "FlareLogApi.h"
#include "../../Flare.h"
#include "FlareLogWriter.h"

#include "../FlareCompany.h"
#include "../../Spacecrafts/FlareBomb.h"
#include "../../Spacecrafts/FlareWeapon.h"
#include "../../Spacecrafts/FlareSpacecraft.h"
#include "../FlareSimulatedSector.h"
#include "../../Spacecrafts/FlareSimulatedSpacecraft.h"
#include "../Save/FlareSaveWriter.h"

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
								UFlareSimulatedSector* ConstructionSector,
								FFlareSpacecraftDescription* ConstructionStationDescription,
								UFlareSimulatedSpacecraft* ConstructionStation)
{
	FlareLogMessage Message;
	Message.Target = EFlareLogTarget::Game;
	Message.Event = EFlareLogEvent::AI_CONSTRUCTION_STARTED;

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = Company->GetShortName().ToString();
		Message.Params.Add(Param);
	}
	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = ConstructionSector->GetIdentifier().ToString();
		Message.Params.Add(Param);
	}
	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = ConstructionStationDescription->Identifier.ToString();
		Message.Params.Add(Param);
	}
	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = (ConstructionStation ? ConstructionStation->GetImmatriculation().ToString() : "");
		Message.Params.Add(Param);
	}

	FFlareLogWriter::PushWriterMessage(Message);
}

void GameLog::UnlockResearch(UFlareCompany* Company,
								FFlareTechnologyDescription* Research)
{
	FlareLogMessage Message;
	Message.Target = EFlareLogTarget::Game;
	Message.Event = EFlareLogEvent::COMPANY_UNLOCK_RESEARCH;

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = Company->GetShortName().ToString();
		Message.Params.Add(Param);
	}
	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = Research->Identifier.ToString();
		Message.Params.Add(Param);
	}

	FFlareLogWriter::PushWriterMessage(Message);
}

// Combat log api

void CombatLog::SectorActivated(UFlareSimulatedSector* Sector)
{
	FlareLogMessage Message;
	Message.Target = EFlareLogTarget::Combat;
	Message.Event = EFlareLogEvent::SECTOR_ACTIVATED;

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = Sector->GetIdentifier().ToString();
		Message.Params.Add(Param);
	}

	FFlareLogWriter::PushWriterMessage(Message);
}

void CombatLog::SectorDeactivated(UFlareSimulatedSector* Sector)
{
	FlareLogMessage Message;
	Message.Target = EFlareLogTarget::Combat;
	Message.Event = EFlareLogEvent::SECTOR_DEACTIVATED;

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = Sector->GetIdentifier().ToString();
		Message.Params.Add(Param);
	}

	FFlareLogWriter::PushWriterMessage(Message);
}

void CombatLog::AutomaticBattleStarted(UFlareSimulatedSector* Sector)
{
	FlareLogMessage Message;
	Message.Target = EFlareLogTarget::Combat;
	Message.Event = EFlareLogEvent::AUTOMATIC_BATTLE_STARTED;

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = Sector->GetIdentifier().ToString();
		Message.Params.Add(Param);
	}

	FFlareLogWriter::PushWriterMessage(Message);
}

void CombatLog::AutomaticBattleEnded(UFlareSimulatedSector* Sector)
{
	FlareLogMessage Message;
	Message.Target = EFlareLogTarget::Combat;
	Message.Event = EFlareLogEvent::AUTOMATIC_BATTLE_ENDED;

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = Sector->GetIdentifier().ToString();
		Message.Params.Add(Param);
	}

	FFlareLogWriter::PushWriterMessage(Message);
}

void CombatLog::BombDropped(AFlareBomb *Bomb)
{
	FlareLogMessage Message;
	Message.Target = EFlareLogTarget::Combat;
	Message.Event = EFlareLogEvent::BOMB_DROPPED;

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = Bomb->GetIdentifier().ToString();
		Message.Params.Add(Param);
	}

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = Bomb->GetFiringSpacecraft()->GetImmatriculation().ToString();
		Message.Params.Add(Param);
	}

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = Bomb->GetFiringWeapon()->Save()->ShipSlotIdentifier.ToString();
		Message.Params.Add(Param);
	}

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = Bomb->GetFiringWeapon()->GetDescription()->Identifier.ToString();
		Message.Params.Add(Param);
	}

	FFlareLogWriter::PushWriterMessage(Message);
}

void CombatLog::BombDestroyed(FName BombIdentifier)
{
	FlareLogMessage Message;
	Message.Target = EFlareLogTarget::Combat;
	Message.Event = EFlareLogEvent::BOMB_DESTROYED;

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = BombIdentifier.ToString();
		Message.Params.Add(Param);
	}

	FFlareLogWriter::PushWriterMessage(Message);
}

void CombatLog::SpacecraftDamaged(UFlareSimulatedSpacecraft* Spacecraft, float Energy, float Radius, FVector RelativeLocation, EFlareDamage::Type DamageType, UFlareCompany* DamageSource, FString DamageCauser)
{
	FlareLogMessage Message;
	Message.Target = EFlareLogTarget::Combat;
	Message.Event = EFlareLogEvent::SPACECRAFT_DAMAGED;

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = Spacecraft->GetImmatriculation().ToString();
		Message.Params.Add(Param);
	}

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = UFlareSaveWriter::FormatEnum<EFlareDamage::Type>("EFlareDamage", DamageType);
		Message.Params.Add(Param);
	}

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::Float;
		Param.FloatValue = Energy;
		Message.Params.Add(Param);
	}

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::Float;
		Param.FloatValue = Radius;
		Message.Params.Add(Param);
	}

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::Vector3;
		Param.Vector3Value = RelativeLocation;
		Message.Params.Add(Param);
	}

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue =  (DamageSource ? DamageSource->GetShortName().ToString() : "");
		Message.Params.Add(Param);
	}

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = DamageCauser;
		Message.Params.Add(Param);
	}

	FFlareLogWriter::PushWriterMessage(Message);
}

void CombatLog::SpacecraftComponentDamaged(UFlareSimulatedSpacecraft* Spacecraft, FFlareSpacecraftComponentSave* ComponentData, FFlareSpacecraftComponentDescription* ComponentDescription, float Energy, float EffectiveEnergy, EFlareDamage::Type DamageType, float InitialDamageRatio, float TerminalDamageRatio)
{
	FlareLogMessage Message;
	Message.Target = EFlareLogTarget::Combat;
	Message.Event = EFlareLogEvent::SPACECRAFT_COMPONENT_DAMAGED;

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = Spacecraft->GetImmatriculation().ToString();
		Message.Params.Add(Param);
	}

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = ComponentData->ShipSlotIdentifier.ToString();
		Message.Params.Add(Param);
	}

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = ComponentDescription->Identifier.ToString();
		Message.Params.Add(Param);
	}

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::Float;
		Param.FloatValue = Energy;
		Message.Params.Add(Param);
	}

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::Float;
		Param.FloatValue = EffectiveEnergy;
		Message.Params.Add(Param);
	}

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = UFlareSaveWriter::FormatEnum<EFlareDamage::Type>("EFlareDamage", DamageType);
		Message.Params.Add(Param);
	}

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::Float;
		Param.FloatValue = InitialDamageRatio;
		Message.Params.Add(Param);
	}

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::Float;
		Param.FloatValue = TerminalDamageRatio;
		Message.Params.Add(Param);
	}

	FFlareLogWriter::PushWriterMessage(Message);
}

void CombatLog::SpacecraftHarpooned(UFlareSimulatedSpacecraft* Spacecraft, UFlareCompany* HarpoonOwner)
{
	FlareLogMessage Message;
	Message.Target = EFlareLogTarget::Combat;
	Message.Event = EFlareLogEvent::SPACECRAFT_HARPOONED;

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue = Spacecraft->GetImmatriculation().ToString();
		Message.Params.Add(Param);
	}

	{
		FlareLogMessageParam Param;
		Param.Type = EFlareLogParam::String;
		Param.StringValue =  (HarpoonOwner ? HarpoonOwner->GetShortName().ToString() : "");
		Message.Params.Add(Param);
	}

	FFlareLogWriter::PushWriterMessage(Message);
}
