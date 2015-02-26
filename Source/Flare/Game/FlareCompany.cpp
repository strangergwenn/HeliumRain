
#include "Flare.h"
#include "FlareCompany.h"


/*----------------------------------------------------
	Constructor
----------------------------------------------------*/

UFlareCompany::UFlareCompany(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


/*----------------------------------------------------
	Save
----------------------------------------------------*/

void UFlareCompany::Load(const FFlareCompanySave& Data)
{
	CompanyData = Data;
	CompanyData.Identifier = FName(*GetName());
}

FFlareCompanySave* UFlareCompany::Save()
{
	

	return &CompanyData;
}
