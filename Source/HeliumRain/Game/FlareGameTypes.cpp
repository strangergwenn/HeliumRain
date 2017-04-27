
#include "FlareGameTypes.h"
#include "../Flare.h"

#define LOCTEXT_NAMESPACE "FlareNavigationHUD"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareGameTypes::UFlareGameTypes(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


/*----------------------------------------------------
	Text
----------------------------------------------------*/

FText UFlareGameTypes::GetCombatGroupDescription(EFlareCombatGroup::Type Type)
{
	FText Result;

	switch (Type)
	{
		case EFlareCombatGroup::AllMilitary:   Result = LOCTEXT("AllMilitary",  "All military ships");   break;
		case EFlareCombatGroup::Capitals:      Result = LOCTEXT("AllCapitals",  "Capital ships");        break;
		case EFlareCombatGroup::Fighters:      Result = LOCTEXT("AllFighters",  "Fighters");             break;
		case EFlareCombatGroup::Civilan:       Result = LOCTEXT("AllCivilians", "Freighters");           break;
	}

	return Result;
}

FText UFlareGameTypes::GetCombatTacticDescription(EFlareCombatTactic::Type Type)
{
	FText Result;

	switch (Type)
	{
		case EFlareCombatTactic::ProtectMe:        Result = LOCTEXT("ProtectMe",       "Protect me");         break;
		case EFlareCombatTactic::AttackMilitary:   Result = LOCTEXT("AttackMilitary",  "Attack military");    break;
		case EFlareCombatTactic::AttackStations:   Result = LOCTEXT("AttackStations",  "Attack stations");    break;
		case EFlareCombatTactic::AttackCivilians:  Result = LOCTEXT("AttackCivilians", "Attack freighters");  break;
		case EFlareCombatTactic::StandDown:        Result = LOCTEXT("StandDown",       "Stand down");         break;
	}

	return Result;
}

const FSlateBrush* UFlareGameTypes::GetCombatGroupIcon(EFlareCombatGroup::Type Type)
{
	const FSlateBrush* Result = NULL;

	switch (Type)
	{
		case EFlareCombatGroup::AllMilitary:   Result = FFlareStyleSet::GetIcon("AllMilitary");   break;
		case EFlareCombatGroup::Capitals:      Result = FFlareStyleSet::GetIcon("AllCapitals");   break;
		case EFlareCombatGroup::Fighters:      Result = FFlareStyleSet::GetIcon("AllFighters");   break;
		case EFlareCombatGroup::Civilan:       Result = FFlareStyleSet::GetIcon("AllFreighters"); break;
	}

	return Result;
}

/*----------------------------------------------------
	Float buffer
----------------------------------------------------*/

void FFlareFloatBuffer::Init(int32 Size)
{
	MaxSize = Size;
	Values.Empty(MaxSize);
	WriteIndex = 0;
}

void FFlareFloatBuffer::Resize(int32 Size)
{
	if(Size <= Values.Num())
	{
		TArray<float> NewValues;
		for (int Age = Size-1; Age >= 0; Age--)
		{
			NewValues.Add(GetValue(Age));
		}
		// Override
		Values = NewValues;
		WriteIndex = 0;
	}

	MaxSize = Size;
}

void FFlareFloatBuffer::Append(float NewValue)
{
	if(Values.Num() <= WriteIndex)
	{
		Values.Add(NewValue);
		WriteIndex = Values.Num();
	}
	else
	{
		Values[WriteIndex] = NewValue;
		WriteIndex++;
	}

	if(WriteIndex >= MaxSize)
	{
		WriteIndex = 0;
	}
}

float FFlareFloatBuffer::GetValue(int32 Age)
{
	if(Values.Num() == 0)
	{
		return 0.f;
	}

	if(Age >= Values.Num())
	{
		Age = Values.Num() - 1;
	}

	int32 ReadIndex = WriteIndex - 1 - Age;
	if (ReadIndex < 0)
	{
		ReadIndex += Values.Num();
	}

	return Values[ReadIndex];
}

float FFlareFloatBuffer::GetMean(int32 StartAge, int32 EndAge)
{
	float Count = 0.f;
	float Sum = 0.f;

	if(StartAge >= Values.Num())
	{
		StartAge = Values.Num() - 1;
	}

	if(EndAge >= Values.Num())
	{
		EndAge = Values.Num() - 1;
	}

	for (int Age = StartAge; Age <= EndAge; Age++)
	{
		Sum += GetValue(Age);
		Count += 1.f;
	}

	if(Count == 0.f)
	{
		return 0.f;
	}
	return Sum/Count;
}

bool FFlareBundle::HasFloat(FName Key) const
{
	return FloatValues.Contains(Key);
}

bool FFlareBundle::HasInt32(FName Key) const
{
	return Int32Values.Contains(Key);
}

bool FFlareBundle::HasTransform(FName Key) const
{
	return TransformValues.Contains(Key);
}

bool FFlareBundle::HasVectorArray(FName Key) const
{
	return VectorArrayValues.Contains(Key);
}

bool FFlareBundle::HasName(FName Key) const
{
	return NameValues.Contains(Key);
}

bool FFlareBundle::HasNameArray(FName Key) const
{
	return NameArrayValues.Contains(Key);
}

bool FFlareBundle::HasString(FName Key) const
{
	return StringValues.Contains(Key);
}

bool FFlareBundle::HasTag(FName Tag) const
{
	return Tags.Contains(Tag);
}


float FFlareBundle::GetFloat(FName Key, float Default) const
{
	if(FloatValues.Contains(Key))
	{
		return FloatValues[Key];
	}
	return Default;
}

int32 FFlareBundle::GetInt32(FName Key, int32 Default) const
{
	if(Int32Values.Contains(Key))
	{
		return Int32Values[Key];
	}
	return Default;
}

FTransform FFlareBundle::GetTransform(FName Key, const FTransform Default) const
{
	if(TransformValues.Contains(Key))
	{
		return TransformValues[Key];
	}
	return Default;
}

TArray<FVector> FFlareBundle::GetVectorArray(FName Key) const
{
	if(VectorArrayValues.Contains(Key))
	{
		return VectorArrayValues[Key].Entries;
	}
	return TArray<FVector>();
}

FName FFlareBundle::GetName(FName Key) const
{
	if(NameValues.Contains(Key))
	{
		return NameValues[Key];
	}
	return "";
}

TArray<FName> FFlareBundle::GetNameArray(FName Key) const
{
	if(NameArrayValues.Contains(Key))
	{
		return NameArrayValues[Key].Entries;
	}
	return TArray<FName>();
}

FString FFlareBundle::GetString(FName Key) const
{
	if (StringValues.Contains(Key))
	{
		return StringValues[Key];
	}
	return "";
}

FFlareBundle& FFlareBundle::PutFloat(FName Key, float Value)
{
	FloatValues.Add(Key, Value);
	return *this;
}

FFlareBundle& FFlareBundle::PutInt32(FName Key, int32 Value)
{
	Int32Values.Add(Key, Value);
	return *this;
}

FFlareBundle& FFlareBundle::PutTransform(FName Key, const FTransform Value)
{
	TransformValues.Add(Key, Value);
	return *this;
}

FFlareBundle& FFlareBundle::PutVectorArray(FName Key, const TArray<FVector> Value)
{
	FVectorArray Array;
	Array.Entries = Value;
	VectorArrayValues.Add(Key, Array);
	return *this;
}

FFlareBundle& FFlareBundle::PutName(FName Key, FName Value)
{
	NameValues.Add(Key, Value);
	return *this;
}

FFlareBundle& FFlareBundle::PutNameArray(FName Key, const TArray<FName> Value)
{
	FNameArray Array;
	Array.Entries = Value;
	NameArrayValues.Add(Key, Array);
	return *this;
}

FFlareBundle& FFlareBundle::PutString(FName Key, FString Value)
{
	StringValues.Add(Key, Value);
	return *this;
}

FFlareBundle& FFlareBundle::PutTag(FName Tag)
{
	Tags.Add(Tag);
	return *this;
}

void FFlareBundle::Clear()
{
	FloatValues.Empty();
	Int32Values.Empty();
	TransformValues.Empty();
	VectorArrayValues.Empty();
	NameValues.Empty();
	NameArrayValues.Empty();
	StringValues.Empty();
	Tags.Empty();
}



#undef LOCTEXT_NAMESPACE
