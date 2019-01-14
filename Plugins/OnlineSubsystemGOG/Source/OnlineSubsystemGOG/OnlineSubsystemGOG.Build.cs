using System.IO;
using UnrealBuildTool;

public class OnlineSubsystemGOG : ModuleRules
{
	public OnlineSubsystemGOG(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				"OnlineSubsystemGOG/Public"
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
				"OnlineSubsystemGOG/Private"
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"OnlineSubsystemUtils"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Projects",
				"Engine",
				"Sockets",
				"OnlineSubsystem",
				"Json",
				"GalaxySDK",
				"PacketHandler"
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}
