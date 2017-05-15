// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
using System;
using System.IO;
using UnrealBuildTool;

public class Omniverse : ModuleRules
{
	public Omniverse(TargetInfo Target)
	{
		
		PublicIncludePaths.AddRange(
			new string[] {
				"Omniverse/Public"
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"Omniverse/Private",
				// ... add other private include paths required here ...
                //"../../../../../share"
            }
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"UMG",
                "HeadMountedDisplay",
                "InputCore",
				"InputDevice",
                "Json",
                "XmlParser",
                "CustomMeshComponent",				
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
		
		string Plugin_dir = Path.Combine(ModuleDirectory, "../../");
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string binaries_dir = Path.Combine(Plugin_dir, "DLL/Win64");
            RuntimeDependencies.Add(new RuntimeDependency(binaries_dir + "/desktop_plugin.dll"));
		}

        DirectoryInfo TheFolder = new DirectoryInfo(Plugin_dir + "/Content/HvSDK");
        foreach (FileInfo NextFile in TheFolder.GetFiles()) {
            RuntimeDependencies.Add(new RuntimeDependency(Plugin_dir + "/Content/HvSDK/" + NextFile.Name));
        }
		
        LoadYourThirdPartyLibraries(Target);
    }

    public bool LoadYourThirdPartyLibraries(TargetInfo Target)
    {
        string ModulePath = ModuleDirectory;
		string Plugin_dir = Path.Combine(ModuleDirectory, "../../");
        //string ModulePath = Path.GetDirectoryName(RulesCompiler.GetModuleFilename(this.GetType().Name));
        string MyLibraryPath = Path.Combine(ModulePath, "ThirdParty", "hidapi/windows/x64/Release");
		PublicIncludePaths.Add(Path.Combine(MyLibraryPath));
        //PublicAdditionalLibraries.Add(Path.Combine(MyLibraryPath, "hidapi.lib"));
        return true;
    }


    public string GetUProjectPath()
    {
        return Path.Combine(ModuleDirectory, "../../../../");
    }
}
