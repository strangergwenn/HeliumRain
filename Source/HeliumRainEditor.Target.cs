// Helium Rain Project - http://helium-rain.com

using UnrealBuildTool;
using System.Collections.Generic;

public class HeliumRainEditorTarget : TargetRules
{
	public HeliumRainEditorTarget(TargetInfo Target) : base(Target)
    {
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.Add("HeliumRain");
        ExtraModuleNames.Add("HeliumRainLoadingScreen");
    }
}
