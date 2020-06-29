#pragma once

#include <SlateBasics.h>
#include <Slate/SlateGameResources.h>
#include "HeliumRain/UI/Style/FlareWidgetStyleCatalog.h"


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

	/** Get the game's default theme */
	static const FFlareStyleCatalog& GetDefaultTheme()
	{
		return FFlareStyleSet::Get().GetWidgetStyle<FFlareStyleCatalog>("/Style/DefaultTheme");
	}

	/** Get an icon */
	static const FSlateBrush* GetIcon(FString Name)
	{
		FString Path = "/Brushes/SB_Icon_" + Name;
		return FFlareStyleSet::Get().GetBrush(*Path);
	}

	/** Get an image */
	static const FSlateBrush* GetImage(FString Name)
	{
		FString Path = "/Brushes/SB_" + Name;
		return FFlareStyleSet::Get().GetBrush(*Path);
	}

	/** Get a color for the current health of something */
	static FLinearColor GetHealthColor(float Health, bool WithAlpha = false);


protected:

	/*----------------------------------------------------
		Data
	----------------------------------------------------*/
	
	/** Resource pointer */
	UPROPERTY()
	static TSharedPtr<FSlateStyleSet> Instance;


};
