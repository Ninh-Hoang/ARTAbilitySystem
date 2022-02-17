// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ART : ModuleRules
{
	public ART(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		MinFilesUsingPrecompiledHeaderOverride = 1;
		
		if (Target.Configuration != UnrealTargetConfiguration.Shipping) bUseUnity = false;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Navmesh",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"UMG",
				"Slate",
				"SlateCore",
				"InputCore",
				"AIModule",
				"PhysicsCore",
				"GameplayAbilities",
				"GameplayTags",
				"GameplayTasks",
				"Paper2D",
				"NavigationSystem",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
				"Networking",
				"TheCurviestCurve",
				// ... add private dependencies that you statically link with here ...	
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
