
#pragma once
#include "FlareResource.generated.h"

/** Spacecraft cargo save data */
USTRUCT()
struct FFlareCargoSave
{
	GENERATED_USTRUCT_BODY()

	/** Cargo resource */
	UPROPERTY(EditAnywhere, Category = Save)
	FName ResourceIdentifier;

	/** Cargo quantity */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 Quantity;
};

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
	uint32 Quantity;

	/** Cargo capacity */
	UPROPERTY(EditAnywhere, Category = Save)
	uint32 Capacity;
};

UCLASS()
class FLARE_API UFlareResource : public UObject
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
