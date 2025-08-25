// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectX : ModuleRules
{
	public ProjectX(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				"ProjectX"
			}
		);
		
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"GameplayTags",
			"UMG",
			"Slate",
			"SlateCore",
			"NetCore",
			"GameplayAbilities", 
			"GameplayTags", 
			"GameplayTasks"
        });
		
		// For Debug
		if (Target.Configuration == UnrealTargetConfiguration.DebugGame)
		{
			OptimizeCode = CodeOptimization.Never;
		}
	}
}
