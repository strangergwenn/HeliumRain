// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HeliumRain : ModuleRules
{
	public HeliumRain(TargetInfo Target)
	{
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
                "JsonUtilities"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
				"InputCore",
				"Slate",
				"SlateCore",
				"HeliumRainLoadingScreen",
			}
        );

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
	}
}
