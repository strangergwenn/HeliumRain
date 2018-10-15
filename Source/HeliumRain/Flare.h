#pragma once

#include "EngineMinimal.h"


/*----------------------------------------------------
	Logging
----------------------------------------------------*/

DECLARE_LOG_CATEGORY_EXTERN(LogFlare, Log, All);

#define FLOG(Format)        UE_LOG(LogFlare, Display, TEXT(Format))
#define FLOGV(Format, ...)  UE_LOG(LogFlare, Display, TEXT(Format), __VA_ARGS__)

DECLARE_STATS_GROUP(TEXT("HeliumRain"), STATGROUP_Flare, STATCAT_Advanced);


/*----------------------------------------------------
	Tools
----------------------------------------------------*/

// Moving average
template<class T>
struct FFlareMovingAverage
{
	FFlareMovingAverage()
	{
		SetSize(60);
	}

	void SetSize(int32 NewSize)
	{
		Size = NewSize;

		while (Values.Num() > Size)
		{
			Values.Pop();
		}

		Update();
	}

	void Add(T Value)
	{
		Values.Insert(Value, 0);

		while (Values.Num() > Size)
		{
			Values.Pop();
		}

		Update();
	}

	T Get() const
	{
		return Average;
	}

	void Clear()
	{
		Values.Empty();
		Update();
	}

	void Update()
	{
		T Total = 0 * T();

		for (T Value : Values)
		{
			Total += Value;
		}

		if (Values.Num() > 0)
		{
			float SizeDivider = 1.0f / Values.Num();
			Average = Total * SizeDivider;
		}
		else
		{
			Average = 0 * T();
		}
	}

private:

	T          Average;
	TArray<T>  Values;
	int32      Size;
};


/*----------------------------------------------------
	Error reporting
----------------------------------------------------*/

#define FCHECK(Expression)  do \
	{ \
		if (!ensure(Expression)) \
		{ \
			FFlareModule::ReportError(__FUNCTION__); \
		} \
	} \
	while (0)


/*----------------------------------------------------
	Game module definition
----------------------------------------------------*/

class FFlareModule : public FDefaultGameModuleImpl
{

public:

	void StartupModule() override;

	void ShutdownModule() override;

	/** Send a crash report, display the stack and exit */
	static void ReportError(FString Title);

protected:

	static class FFlareStyleSet* StyleInstance;

};

