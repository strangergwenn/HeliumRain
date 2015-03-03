// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Flare : ModuleRules
{
	public Flare(TargetInfo Target)
	{
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "OnlineSubsystem", "OnlineSubsystemUtils" });
        PrivateDependencyModuleNames.AddRange(new string[] { "FlareLoadingScreen", "Slate", "SlateCore" });
        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
	}
}
