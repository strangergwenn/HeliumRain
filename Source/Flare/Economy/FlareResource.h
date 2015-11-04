
#pragma once
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
