
#include "FlareLoadingScreen.h"

#include "SlateBasics.h"
#include "SlateExtras.h"
#include "MoviePlayer.h"
#include "SThrobber.h"

#define LOCTEXT_NAMESPACE "FlareLoadingScreen"


/*----------------------------------------------------
	Dynamic brush
----------------------------------------------------*/

struct FFlareLoadingScreenBrush : public FSlateDynamicImageBrush, public FGCObject
{
	FFlareLoadingScreenBrush( const FName InTextureName, const FVector2D& InImageSize )
		: FSlateDynamicImageBrush( InTextureName, InImageSize )
	{
		SetResourceObject(LoadObject<UObject>(NULL, *InTextureName.ToString()));
	}

	virtual void AddReferencedObjects(FReferenceCollector& Collector)
	{
		FSlateBrush::AddReferencedObjects(Collector);
	}
};


/*----------------------------------------------------
	Screen layout
----------------------------------------------------*/

class SFlareLoadingScreen : public SCompoundWidget
{

public:

	SLATE_BEGIN_ARGS(SFlareLoadingScreen){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		// Get brush data
		static const FName LoadingScreenName(TEXT("/Engine/EngineResources/Black.Black"));
		static const FName ThrobberImageName(TEXT("/Game/Slate/Images/TX_Image_LargeButtonInvertedBackground.TX_Image_LargeButtonInvertedBackground"));

		// Create textures
		LoadingScreenBrush = MakeShareable(new FFlareLoadingScreenBrush(LoadingScreenName, FVector2D(1920, 1080)));
		ThrobberBrush = MakeShareable(new FFlareLoadingScreenBrush(ThrobberImageName, FVector2D(64, 64)));

		// Structure
		ChildSlot
		[
			SNew(SOverlay)

			// Background image
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SImage)
				.Image(LoadingScreenBrush.Get())
			]

			// Data box
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SVerticalBox)

				// Title
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Bottom)
				.HAlign(HAlign_Center)
				.Padding(FMargin(10.0f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("LoadingTitle", "HELIUM RAIN"))
					.Font(FSlateFontInfo(FPaths::ProjectContentDir() / TEXT("Slate/Fonts/Lato700.ttf"), 72))
				]

				// Throbber
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Top)
				.HAlign(HAlign_Center)
				.Padding(FMargin(10.0f))
				[
					SNew(SThrobber)
					.PieceImage(ThrobberBrush.Get())
					.NumPieces(5)
				]
			]
		];
	}

private:
	
	// Slate data
	TSharedPtr<FSlateDynamicImageBrush> ThrobberBrush;
	TSharedPtr<FSlateDynamicImageBrush> LoadingScreenBrush;


};


/*----------------------------------------------------
	Game submodule
----------------------------------------------------*/

class FFlareLoadingScreenModule : public IFlareLoadingScreenModule
{

public:

	virtual void StartupModule() override
	{
		if (IsMoviePlayerEnabled())
		{
			CreateScreen();
		}
	}
	
	virtual bool IsGameModule() const override
	{
		return true;
	}

	virtual void StartInGameLoadingScreen() override
	{
		CreateScreen();

		GetMoviePlayer()->PlayMovie();
	}

	virtual void CreateScreen()
	{
		FLoadingScreenAttributes LoadingScreen;
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
		LoadingScreen.WidgetLoadingScreen = SNew(SFlareLoadingScreen);
		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	}

};

#undef LOCTEXT_NAMESPACE
IMPLEMENT_GAME_MODULE(FFlareLoadingScreenModule, HeliumRainLoadingScreen);
