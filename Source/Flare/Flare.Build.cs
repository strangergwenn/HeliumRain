// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Flare : ModuleRules
{
	public Flare(TargetInfo Target)
	{
        PublicDependencyModuleNames.AddRange(
            new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
			}
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
				"InputCore",
				"Slate",
				"SlateCore",
				"FlareLoadingScreen",
			}
        );

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
	}
}
