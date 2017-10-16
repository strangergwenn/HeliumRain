// Helium Rain Project - http://helium-rain.com

using UnrealBuildTool;

public class HeliumRain : ModuleRules
{
	public HeliumRain(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
                "RHI",
                "RenderCore",
                "Json",
                "JsonUtilities",
                "ApexDestruction"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
				"InputCore",
				"Slate",
				"SlateCore",
				"HeliumRainLoadingScreen",
                "AppFramework"
            }
        );

        PublicIncludePaths.Add("ApexDestruction/Source/ApexDestruction/Public");
        
        PrivateDependencyModuleNames.Add("OnlineSubsystem");
        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
    }
}
