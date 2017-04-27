// Helium Rain Project - http://helium-rain.com

using UnrealBuildTool;
using System.IO;

public class InternalComponentVisualizer : ModuleRules
{
    public InternalComponentVisualizer(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateIncludePaths.AddRange(new string[] { "InternalComponentVisualizer/Private" });
        PublicIncludePaths.AddRange(new string[] { "InternalComponentVisualizer/Public" });

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeliumRain" });
        PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
    }
}
