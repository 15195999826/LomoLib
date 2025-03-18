using UnrealBuildTool;

public class LomoLibEditor : ModuleRules
{
    public LomoLibEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "UnrealEd","InputCore"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "Json",
                "JsonUtilities",
                "DesktopPlatform",
                "ClassViewer",
                "PropertyEditor",
                "ToolMenus",
            }
        );
    }
}