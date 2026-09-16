// Copyright (c) 2025, Fools With Blades. All rights reserved.

using UnrealBuildTool;

public class CustomEditor : ModuleRules
{
	public CustomEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "EmbersOath", "Engine", "UnrealEd", "Json", "JsonUtilities" });
	}
}