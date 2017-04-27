// Helium Rain Project - http://helium-rain.com

using UnrealBuildTool;
using System.Collections.Generic;

public class HeliumRainTarget : TargetRules
{
	public HeliumRainTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        bUsesSteam = true;
        ExtraModuleNames.Add("HeliumRain");
    }
}
