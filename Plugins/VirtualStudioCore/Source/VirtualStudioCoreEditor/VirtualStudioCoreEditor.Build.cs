using UnrealBuildTool;

public class VirtualStudioCoreEditor : ModuleRules
{
    public VirtualStudioCoreEditor(ReadOnlyTargetRules Target)
        : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "LevelEditor",
                "Settings",
                "Slate",
                "SlateCore",
                "ToolMenus",
                "UnrealEd",
                "VirtualStudioCore"
            }
        );
    }
}
