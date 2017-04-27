// Helium Rain Project - http://helium-rain.com

using UnrealBuildTool;
using System.Collections.Generic;

public class HeliumRainEditorTarget : TargetRules
{
	public HeliumRainEditorTarget(TargetInfo Target) : base(Target)
    {
		Type = TargetType.Editor;
        ExtraModuleNames.Add("HeliumRain");
        ExtraModuleNames.Add("HeliumRainLoadingScreen");
    }
}
