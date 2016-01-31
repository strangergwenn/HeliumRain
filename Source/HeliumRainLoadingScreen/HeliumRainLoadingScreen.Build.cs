// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

// This module must be loaded "PreLoadingScreen" in the .uproject file, otherwise it will not hook in time!

public class HeliumRainLoadingScreen : ModuleRules
{
    public HeliumRainLoadingScreen(TargetInfo Target)
	{
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
