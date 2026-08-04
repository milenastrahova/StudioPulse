using UnrealBuildTool;

public class StudioPulse : ModuleRules
{
    public StudioPulse(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.Add("VirtualStudioCore");
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "UMG"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore",
                "HTTP",
                "Json",
                "JsonUtilities",
                "WebSockets"
            }
        );
    }
}
