// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FlareLoadingScreen : ModuleRules
{
    public FlareLoadingScreen(TargetInfo Target)
	{
		PrivateIncludePaths.Add("../Source/FlareLoadingScreen");
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
        PrivateDependencyModuleNames.AddRange(new string[] { "MoviePlayer", "Slate", "SlateCore" });
	}
}
