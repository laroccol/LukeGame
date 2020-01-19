// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LukeGame : ModuleRules
{
	public LukeGame(ReadOnlyTargetRules Target) : base(Target)
	{
        bEnableExceptions = true;

		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "PhysX", "APEX", "CableComponent", "GameLiftServerSDK",
            "OnlineSubsystem", "OnlineSubsystemUtils", "GameplayAbilities", "GameplayTasks", "GameplayTags" });

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");

        PrivateDependencyModuleNames.AddRange(new string[] { "CableComponent" });

    }
}
