// Helium Rain Project - http://helium-rain.com

using UnrealBuildTool;

public class HeliumRainLoadingScreen : ModuleRules
{
    public HeliumRainLoadingScreen(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateIncludePaths.Add("../Source/HeliumRainLoadingScreen");

        PrivateDependencyModuleNames.AddRange(
            new string[] {
				"Core",
				"CoreUObject",
				"MoviePlayer",
				"Slate",
				"SlateCore",
				"InputCore"
			}
        );
	}
}
