#pragma once

#include "SlateBasics.h"
#include "SlateGameResources.h"


class FFlareStyleSet
{

public:

	/*----------------------------------------------------
		Public methods
	----------------------------------------------------*/

	/** Setup the game resources */
	static void Initialize();

	/** Remove the game resources */
	static void Shutdown();

	/** Get a reference to the current data */
	static const ISlateStyle& Get()
	{
		return *Instance;
	}

	/** Setup resources (internal) */
	static TSharedRef< FSlateStyleSet > Create();

	/** Get an icon */
	static const FSlateBrush* GetIcon(FString Name)
	{
		FString Path = "/Brushes/SB_Icon_" + Name;
		return FFlareStyleSet::Get().GetBrush(*Path);
	}

	/** Get the color for enemies */
	static FLinearColor GetEnemyColor()
	{
		return (FLinearColor::Red).Desaturate(0.05);
	}

	/** Get the color for friendlies */
	static FLinearColor GetFriendlyColor()
	{
		return (FLinearColor::Green).Desaturate(0.05);
	}


protected:

	/*----------------------------------------------------
		Data
	----------------------------------------------------*/
	
	/** Resource pointer */
	static TSharedPtr<FSlateStyleSet> Instance;


};
