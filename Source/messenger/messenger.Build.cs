// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class messenger : ModuleRules
{
	public messenger(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "Sockets", "Networking", "Json", "JsonUtilities" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });


		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        string OpenSSLPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty/OpenSSL"));

        PublicIncludePaths.Add(Path.Combine(OpenSSLPath, "include"));

        PublicAdditionalLibraries.Add(Path.Combine(OpenSSLPath, "lib", "libcrypto.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(OpenSSLPath, "lib", "libssl.lib"));
    }
}