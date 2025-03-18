#include "LomoLibEditor.h"
// 在LomoLibEditor.h中添加以下头文件引用
#include "Struct2JsonWidget.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FLomoLibEditorModule"


static void OpenStruct2JsonWindow()
{
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("StructToJsonWindow", "结构体转JSON工具"))
		.ClientSize(FVector2D(600, 700))
		.SupportsMaximize(true)
		.SupportsMinimize(true);

	Window->SetContent(SNew(SStructToJsonWidget));
	FSlateApplication::Get().AddWindow(Window);
}

static void RegisterGameEditorMenus()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu");
	FToolMenuSection& Section = Menu->AddSection("LomLib", TAttribute<FText>(), FToolMenuInsert("Help", EToolMenuInsertType::After));
	Section.AddSubMenu(
		"LomLibSubMenu",
		LOCTEXT("LomLibSubMenu_Label", "LomLib"),
		LOCTEXT("LomLibSubMenu_ToolTip", "This is a submenu under LomLib"),
		FNewToolMenuDelegate::CreateLambda([](UToolMenu* InMenu)
		{
			FToolMenuSection& SubSection = InMenu->AddSection("LomLibSubSection");

			SubSection.AddMenuEntry(
				"LomLibSubMenuEntry1",
				LOCTEXT("LomLibSubMenuEntry1_Label", "Struct To Json"),
				LOCTEXT("LomLibSubMenuEntry1_ToolTip", "将任意结构体转化为Json并保存到指定路径"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&OpenStruct2JsonWindow))
			);
		})
	);
}

void FLomoLibEditorModule::StartupModule()
{
	if (!IsRunningGame())
	{
		if (FSlateApplication::IsInitialized())
		{
			ToolMenusHandle = UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(&RegisterGameEditorMenus));
		}
		
	}
}

void FLomoLibEditorModule::ShutdownModule()
{
	if (UObjectInitialized() && ToolMenusHandle.IsValid())
	{
		UToolMenus::UnRegisterStartupCallback(ToolMenusHandle);
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FLomoLibEditorModule, LomoLibEditor)