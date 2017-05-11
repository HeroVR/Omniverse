// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HVSDKSampleUE4 : ModuleRules
{
	public HVSDKSampleUE4(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "Slate", "UMG", "Omniverse" });
	}
}
