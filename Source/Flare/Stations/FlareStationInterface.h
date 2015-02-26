#pragma once

#include "../Game/FlareCompany.h"
#include "FlareStationInterface.generated.h"


class IFlareShipInterface;
class IFlareStationInterface;


/** Station docking info */
struct FFlareDockingInfo
{
	bool                      Granted;
	bool                      Occupied;
	int32                     DockId;
	IFlareStationInterface*   Station;
	IFlareShipInterface*      Ship;

	FRotator                  Rotation;
	FVector                   StartPoint;
	FVector                   EndPoint;

	FFlareDockingInfo()
		: Granted(false)
		, Occupied(false)
		, DockId(-1)
		, Station(NULL)
	{}
};


/** Game save data */
USTRUCT()
struct FFlareStationSave
{
	GENERATED_USTRUCT_BODY()

	/** Station location */
	UPROPERTY(EditAnywhere, Category = Save)
	FVector Location;

	/** Station rotation */
	UPROPERTY(EditAnywhere, Category = Save)
	FRotator Rotation;

	/** Station name */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Name;

	/** Station catalog identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName Identifier;

	/** Company identifier */
	UPROPERTY(EditAnywhere, Category = Save)
	FName CompanyIdentifier;
};


/** Catalog data structure for a station */
USTRUCT()
struct FFlareStationDescription
{
	GENERATED_USTRUCT_BODY()

	/** Station internal name */
	UPROPERTY(EditAnywhere, Category = Content) FName Identifier;

	/** Station name */
	UPROPERTY(EditAnywhere, Category = Content) FText Name;

	/** Station description */
	UPROPERTY(EditAnywhere, Category = Content) FText Description;
	
	/** Station mesh name */
	UPROPERTY(EditAnywhere, Category = Content) UBlueprint* Template;

	/** Station mesh preview image */
	UPROPERTY(EditAnywhere, Category = Content) FSlateBrush MeshPreviewBrush;

};


/** Interface wrapper */
UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UFlareStationInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};


/** Actual interface */
class IFlareStationInterface
{
	GENERATED_IINTERFACE_BODY()

public:

	/*----------------------------------------------------
		Save
	----------------------------------------------------*/

	/** Load the station from a save file */
	virtual void Load(const FFlareStationSave& Data) = 0;

	/** Save the station to a save file */
	virtual FFlareStationSave* Save() = 0;

	/** Set the parent company */
	virtual void SetOwnerCompany(UFlareCompany* Company) = 0;


	/*----------------------------------------------------
		Station API
	----------------------------------------------------*/

	/** Request a docking point */
	virtual FFlareDockingInfo RequestDock(IFlareShipInterface* Ship) = 0;

	/** Cancel docking */
	virtual void ReleaseDock(IFlareShipInterface* Ship, int32 DockId) = 0;

	/** Confirm the docking */
	virtual void Dock(IFlareShipInterface* Ship, int32 DockId) = 0;

	/** Get the list of docked ships */
	virtual TArray<IFlareShipInterface*> GetDockedShips() = 0;
	

	/*----------------------------------------------------
		Content
	----------------------------------------------------*/

	/** Get a Slate brush */
	static const FSlateBrush* GetIcon(FFlareStationDescription* Characteristic);


};
