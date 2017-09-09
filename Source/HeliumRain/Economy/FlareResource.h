
#pragma once
#include "../Spacecrafts/FlareSpacecraftTypes.h"
#include "FlareResource.generated.h"


/** Resource description */
USTRUCT()
struct FFlareResourceDescription
{
	GENERATED_USTRUCT_BODY()

	/** Name */
	UPROPERTY(EditAnywhere, Category = Content)
	FText Name;

	/** Description */
	UPROPERTY(EditAnywhere, Category = Content)
	FText Description;

	/** Resource identifier */
	UPROPERTY(EditAnywhere, Category = Content)
	FName Identifier;

	/** Acronym */
	UPROPERTY(EditAnywhere, Category = Content)
	FText Acronym;

	/** Resource icon */
	UPROPERTY(EditAnywhere, Category = Content)
	FSlateBrush Icon;

	/** Default resource price */
	UPROPERTY(EditAnywhere, Category = Content)
	int64 DefaultPrice;

	/** Is consumer resource */
	UPROPERTY(EditAnywhere, Category = Content)
	bool IsConsumerResource;

	/** Is maintenance resource */
	UPROPERTY(EditAnywhere, Category = Content)
	bool IsMaintenanceResource;

	/** Display sorting index */
	UPROPERTY(EditAnywhere, Category = Content)
	float DisplayIndex;
};

/** Spacecraft cargo data */
USTRUCT()
struct FFlareCargo
{
	GENERATED_USTRUCT_BODY()

	/** Cargo resource */
	FFlareResourceDescription* Resource;

	/** Cargo quantity */
	UPROPERTY(EditAnywhere, Category = Save)
	int32 Quantity;

	/** Cargo lock */
	UPROPERTY(EditAnywhere, Category = Save)
	TEnumAsByte<EFlareResourceLock::Type> Lock;

	/** Cargo restriction */
	UPROPERTY(EditAnywhere, Category = Save)
	TEnumAsByte<EFlareResourceRestriction::Type> Restriction;

	/** Manual lock */
	UPROPERTY(EditAnywhere, Category = Save)
	bool ManualLock;
};

UCLASS()
class HELIUMRAIN_API UFlareResource : public UObject
{
	GENERATED_UCLASS_BODY()

public:

		/** Name */
		UPROPERTY(EditAnywhere, Category = Content)
		FText Name;

		/** Description */
		UPROPERTY(EditAnywhere, Category = Content)
		FText Description;

		/** Resource identifier */
		UPROPERTY(EditAnywhere, Category = Content)
		FName Identifier;

		/** Acronym */
		UPROPERTY(EditAnywhere, Category = Content)
		FText Acronym;

		/** Resource icon */
		UPROPERTY(EditAnywhere, Category = Content)
		FSlateBrush Icon;

};
