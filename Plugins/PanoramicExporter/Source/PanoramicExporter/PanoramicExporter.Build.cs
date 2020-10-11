// Copyright 2016 Gamemakin LLC. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
    public class PanoramicExporter : ModuleRules
    {
#if WITH_FORWARDED_MODULE_RULES_CTOR
    public PanoramicExporter(ReadOnlyTargetRules Target) : base(Target)
#else
    public PanoramicExporter(TargetInfo Target)
#endif
        {
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

            PrivateIncludePaths.AddRange(
                new string[] {
                    "PanoramicExporter/Private",
                }
            );

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "ImageWrapper",
                    "SlateCore",
                    "RenderCore",
                    "Slate"
                }
            );

            PublicIncludePathModuleNames.Add("Launch");

            if (Target.bBuildEditor == true)
            {
                PrivateDependencyModuleNames.Add("UnrealEd");
            }

#if UE_4_18_OR_LATER
			PublicDefinitions.Add("UE_4_18_OR_LATER=1");
#endif

#if UE_4_20_OR_LATER
			PublicDefinitions.Add("UE_4_20_OR_LATER=1");
#endif
        }
    }
}
