#pragma once

#include "../../Flare.h"


class UFlareFactory;
class UFlareFactory;


class SFlareFactoryInfo : public SCompoundWidget
{
	/*----------------------------------------------------
		Slate arguments
	----------------------------------------------------*/

	SLATE_BEGIN_ARGS(SFlareFactoryInfo)
		: _Factory(NULL)
	{}

	SLATE_ARGUMENT(UFlareFactory*, Factory)
	SLATE_ARGUMENT(TWeakObjectPtr<class AFlareMenuManager>, MenuManager)
	
	SLATE_END_ARGS()


public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Create the widget */
	void Construct(const FArguments& InArgs);

	/** Update this factory's limits */
	void UpdateFactoryLimits();


protected:

	/*----------------------------------------------------
		Content callbacks
	----------------------------------------------------*/

	/** Get the factory name */
	FText GetFactoryName(UFlareFactory* Factory) const;

	/** Get the factory descriptipn */
	FText GetFactoryDescription(UFlareFactory* Factory) const;

	/** Get the factory cycle description */
	FText GetFactoryCycleInfo(UFlareFactory* Factory) const;

	/** Get the factory status info */
	FText GetFactoryStatus(UFlareFactory* Factory) const;
	
	/** Get the current progress */
	TOptional<float> GetProductionProgress(UFlareFactory* Factory) const;

	/** Get the current start visibility */
	EVisibility GetStartProductionVisibility(UFlareFactory* Factory) const;

	/** Get the current stop visibility */
	EVisibility GetStopProductionVisibility(UFlareFactory* Factory) const;
	/*
	EVisibility GetProductionCyclesLimitVisibility(UFlareFactory* Factory) const;*/

	/** Get the current + visibility */
	EVisibility GetIncreaseOutputLimitVisibility(UFlareFactory* Factory, FFlareResourceDescription* Resource) const;

	/** Get the current - visibility */
	EVisibility GetDecreaseOutputLimitVisibility(UFlareFactory* Factory, FFlareResourceDescription* Resource) const;


	/*----------------------------------------------------
		Action callbacks
	----------------------------------------------------*/

	/** Start production */
	void OnStartProduction(UFlareFactory* Factory);

	/** Stop production */
	void OnStopProduction(UFlareFactory* Factory);
	
	/** Decrease the output storage limit */
	void OnDecreaseOutputLimit(UFlareFactory* Factory, FFlareResourceDescription* Resource);

	/** Increase the output storage limit */
	void OnIncreaseOutputLimit(UFlareFactory* Factory, FFlareResourceDescription* Resource);


protected:

	/*----------------------------------------------------
		Private data
	----------------------------------------------------*/

	// Spacecraft data
	UFlareFactory*                                  TargetFactory;

	// Slate data
	TSharedPtr<SVerticalBox>                        LimitList;
	TWeakObjectPtr<class AFlareMenuManager>         MenuManager;


};
