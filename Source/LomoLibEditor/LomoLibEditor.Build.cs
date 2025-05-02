using UnrealBuildTool;

public class LomoLibEditor : ModuleRules
{
    public LomoLibEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "UnrealEd", "InputCore", "LomoLib"
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
                "EditorFramework",
                "ApplicationCore",
                "AssetTools",
                "Projects",
                "Settings",
                "DataTableEditor",
                "MainFrame",
                "EditorSubsystem",
                "EditorWidgets",
                "Blutility",
                "DeveloperSettings",
                "UMG"
            }
        );
    }
}