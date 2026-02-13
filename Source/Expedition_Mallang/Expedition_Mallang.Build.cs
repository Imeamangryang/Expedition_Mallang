// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Expedition_Mallang : ModuleRules
{
	public Expedition_Mallang(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"GeometryFramework",
			"GeometryCore", 
			"DynamicMesh",
			"MeshDescription",
			"StaticMeshDescription",
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Expedition_Mallang",
			"Expedition_Mallang/Variant_Horror",
			"Expedition_Mallang/Variant_Horror/UI",
			"Expedition_Mallang/Variant_Shooter",
			"Expedition_Mallang/Variant_Shooter/AI",
			"Expedition_Mallang/Variant_Shooter/UI",
			"Expedition_Mallang/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
