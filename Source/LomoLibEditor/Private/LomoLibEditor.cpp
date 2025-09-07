#include "LomoLibEditor.h"
#include "ExcelDataTable/ExcelDataTableCommands.h"
#include "ExcelDataTable/ExcelDataTableConverter.h"
#include "ExcelDataTable/ExcelDataTableSettings.h"
#include "ToolMenus.h"
#include "ToolMenuDelegates.h"
#include "LevelEditor.h"
#include "Interfaces/IMainFrameModule.h"
#include "EditorUtilitySubsystem.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "EditorStyleSet.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/AppStyle.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "DesktopPlatformModule.h"
#include "Engine/DataTable.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/FileHelper.h"
#include "Subsystems/UnrealEditorSubsystem.h"
#include "Misc/FeedbackContext.h"
#include "EditorSubsystem.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "PythonBridge.h"
#include "Settings/ProgramAnimationSettings.h"
#include "Animation/ProgramAnimationDataAsset.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "FLomoLibEditorModule"

void FLomoLibEditorModule::StartupModule()
{
	// 注册EditorSubsystem
	// UE5.5.3中不再需要显式注册EditorSubsystem
	// 直接在模块启动时注册命令、设置和菜单
	
	// 注册命令
	RegisterExcelDataTableCommands();
	
	// 注册设置
	RegisterExcelDataTableSettings();
	
	// 注册菜单
	RegisterExcelDataTableMenu();
}

void FLomoLibEditorModule::ShutdownModule()
{
	// 注销菜单
	UnregisterExcelDataTableMenu();
	
	// 注销设置
	UnregisterExcelDataTableSettings();
	
	// 注销命令
	UnregisterExcelDataTableCommands();
}

void FLomoLibEditorModule::RegisterExcelDataTableCommands()
{
	ExcelDataTableCommandsImpl = MakeShareable(new FExcelDataTableCommands());
	ExcelDataTableCommandsImpl->RegisterCommands();
	
	ExcelDataTableCommands = MakeShareable(new FUICommandList);
	
	ExcelDataTableCommands->MapAction(
		ExcelDataTableCommandsImpl->BatchImport,
		FExecuteAction::CreateRaw(this, &FLomoLibEditorModule::BatchImport),
		FCanExecuteAction());
	
	ExcelDataTableCommands->MapAction(
		ExcelDataTableCommandsImpl->BatchExport,
		FExecuteAction::CreateRaw(this, &FLomoLibEditorModule::BatchExport),
		FCanExecuteAction());
	
	ExcelDataTableCommands->MapAction(
		ExcelDataTableCommandsImpl->OpenSettings,
		FExecuteAction::CreateRaw(this, &FLomoLibEditorModule::OpenSettings),
		FCanExecuteAction());
}

void FLomoLibEditorModule::UnregisterExcelDataTableCommands()
{
	ExcelDataTableCommands.Reset();
	ExcelDataTableCommandsImpl.Reset();
}

void FLomoLibEditorModule::RegisterExcelDataTableSettings()
{
	// UDeveloperSettings已经自动注册到Project Settings，不需要手动注册
	// 删除以下代码避免重复显示设置项
	/*
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		// 创建设置节
		ExcelDataTableSettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "ExcelDataTable",
			LOCTEXT("ExcelDataTableSettingsName", "Excel DataTable 转换器"),
			LOCTEXT("ExcelDataTableSettingsDescription", "配置Excel与DataTable之间的映射关系"),
			GetMutableDefault<UExcelDataTableSettings>());
		
		ExcelDataTableSettingsSection->OnModified().BindLambda([]()
		{
			// 保存设置
			GetMutableDefault<UExcelDataTableSettings>()->SaveConfig();
			return true;
		});
	}
	*/
}

void FLomoLibEditorModule::UnregisterExcelDataTableSettings()
{
	// UDeveloperSettings会自动管理注册和注销，不需要手动操作
	/*
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "ExcelDataTable");
	}
	
	ExcelDataTableSettingsSection.Reset();
	*/
}

void FLomoLibEditorModule::RegisterExcelDataTableMenu()
{
	if (FSlateApplication::IsInitialized())
	{
		// 注册菜单
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([this]()
		{
			FToolMenuOwnerScoped OwnerScoped(this);
			
			// 创建LomoLib主菜单
			UToolMenu* MainMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu");
			FToolMenuSection& Section = MainMenu->FindOrAddSection("WindowLayout");
			
			Section.AddSubMenu(
				"LomoLib",
				LOCTEXT("LomoLibMenu", "LomoLib"),
				LOCTEXT("LomoLibMenuTooltip", "LomoLib 工具集"),
				FNewToolMenuDelegate::CreateLambda([this](UToolMenu* Menu)
				{
					// Excel DataTable 转换器
					FToolMenuSection& ExcelSection = Menu->AddSection("ExcelDataTable", LOCTEXT("ExcelDataTableMenuHeader", "Excel DataTable 转换器"));
					ExcelSection.AddMenuEntryWithCommandList(ExcelDataTableCommandsImpl->BatchImport, ExcelDataTableCommands);
					ExcelSection.AddMenuEntryWithCommandList(ExcelDataTableCommandsImpl->BatchExport, ExcelDataTableCommands);
					ExcelSection.AddSeparator(NAME_None);
					ExcelSection.AddMenuEntryWithCommandList(ExcelDataTableCommandsImpl->OpenSettings, ExcelDataTableCommands);

					// 程序动画工具
					FToolMenuSection& AnimationSection = Menu->AddSection("ProgramAnimation", LOCTEXT("ProgramAnimationMenuHeader", "程序动画工具"));
					AnimationSection.AddMenuEntry(
						"RefreshAnimationAssets",
						LOCTEXT("RefreshAnimationAssets", "刷新动画资产"),
						LOCTEXT("RefreshAnimationAssetsTooltip", "扫描并更新动画资产映射"),
						FSlateIcon(),
						FUIAction(FExecuteAction::CreateRaw(this, &FLomoLibEditorModule::RefreshAnimationAssets))
					);
					AnimationSection.AddMenuEntry(
						"GenerateReverseAnimations", 
						LOCTEXT("GenerateReverseAnimations", "生成反转动画"),
						LOCTEXT("GenerateReverseAnimationsTooltip", "根据配置生成反转动画数据"),
						FSlateIcon(),
						FUIAction(FExecuteAction::CreateRaw(this, &FLomoLibEditorModule::GenerateReverseAnimations))
					);
				}),
				false,
				FSlateIcon()
			);
		}));
	}
}

void FLomoLibEditorModule::UnregisterExcelDataTableMenu()
{
	if (UToolMenus::IsToolMenuUIEnabled())
	{
		UToolMenus::UnregisterOwner(this);
	}
	
	// 注销工具栏扩展
	if (ToolbarExtender.IsValid())
	{
		FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor");
		if (LevelEditorModule)
		{
			LevelEditorModule->GetToolBarExtensibilityManager()->RemoveExtender(ToolbarExtender);
		}
		
		ToolbarExtender.Reset();
	}
}

void FLomoLibEditorModule::ImportExcelToDataTable()
{
	// 打开文件对话框，选择Excel文件
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		TArray<FString> OpenFilenames;
		FString DefaultPath = FPaths::ProjectContentDir();
		
		bool bOpened = DesktopPlatform->OpenFileDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
			LOCTEXT("ImportExcelFile", "导入Excel文件").ToString(),
			DefaultPath,
			TEXT(""),
			TEXT("Excel Files (*.xlsx;*.csv)|*.xlsx;*.csv"),
			EFileDialogFlags::None,
			OpenFilenames
		);
		
		if (bOpened && OpenFilenames.Num() > 0)
		{
			// 获取当前选中的DataTable资产
			UDataTable* SelectedDataTable = GetSelectedDataTable();
			
			if (SelectedDataTable)
			{
				// 调用转换器进行导入
				UExcelDataTableConverter* Converter = GEditor->GetEditorSubsystem<UExcelDataTableConverter>();
				FExcelOperationResult Result = Converter->ImportExcelToDataTable(SelectedDataTable->GetPathName(), OpenFilenames[0]);
				
				// 显示结果
				if (Result.bSuccess)
				{
					FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Result.Message));
				}
				else
				{
					FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("导入失败: ") + Result.Message));
				}
			}
			else
			{
				FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoDataTableSelected", "请先在内容浏览器中选择一个DataTable资产"));
			}
		}
	}
}

void FLomoLibEditorModule::ExportDataTableToExcel()
{
	// 获取当前选中的DataTable资产
	UDataTable* SelectedDataTable = GetSelectedDataTable();
	
	if (SelectedDataTable)
	{
		// 打开保存文件对话框
		IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
		if (DesktopPlatform)
		{
			TArray<FString> SaveFilenames;
			FString DefaultPath = FPaths::ProjectSavedDir();
			FString DefaultFile = SelectedDataTable->GetName() + TEXT(".csv");
			
			bool bSaved = DesktopPlatform->SaveFileDialog(
				FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
				LOCTEXT("ExportExcelFile", "导出Excel文件").ToString(),
				DefaultPath,
				DefaultFile,
				TEXT("CSV Files (*.csv)|*.csv|Excel Files (*.xlsx)|*.xlsx"),
				EFileDialogFlags::None,
				SaveFilenames
			);
			
			if (bSaved && SaveFilenames.Num() > 0)
			{
				// 调用转换器进行导出
				UExcelDataTableConverter* Converter = GEditor->GetEditorSubsystem<UExcelDataTableConverter>();
				FExcelOperationResult Result = Converter->ExportDataTableToExcel(SelectedDataTable->GetPathName(), SaveFilenames[0]);
				
				// 显示结果
				if (Result.bSuccess)
				{
					FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Result.Message));
				}
				else
				{
					FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("导出失败: ") + Result.Message));
				}
			}
		}
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoDataTableSelected", "请先在内容浏览器中选择一个DataTable资产"));
	}
}

void FLomoLibEditorModule::BatchImport()
{
	// 调用转换器进行批量导入
	UExcelDataTableConverter* Converter = GEditor->GetEditorSubsystem<UExcelDataTableConverter>();
	TArray<FExcelOperationResult> Results = Converter->BatchProcess(true);
	
	// 显示结果
	FString ResultMessage;
	int32 SuccessCount = 0;
	
	for (const FExcelOperationResult& Result : Results)
	{
		ResultMessage += Result.Message + TEXT("\n");
		if (Result.bSuccess)
		{
			SuccessCount++;
		}
	}
	
	if (Results.Num() > 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(TEXT("批量导入结果:\n%s\n总计: %d 成功, %d 失败"), *ResultMessage, SuccessCount, Results.Num() - SuccessCount)));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoMappingsDefined", "未定义任何映射关系，请先在设置中配置DataTable与Excel的映射"));
	}
}

void FLomoLibEditorModule::BatchExport()
{
	// 调用转换器进行批量导出
	UExcelDataTableConverter* Converter = GEditor->GetEditorSubsystem<UExcelDataTableConverter>();
	TArray<FExcelOperationResult> Results = Converter->BatchProcess(false);
	
	// 显示结果
	FString ResultMessage;
	int32 SuccessCount = 0;
	
	for (const FExcelOperationResult& Result : Results)
	{
		ResultMessage += Result.Message + TEXT("\n");
		if (Result.bSuccess)
		{
			SuccessCount++;
		}
	}
	
	if (Results.Num() > 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(TEXT("批量导出结果:\n%s\n总计: %d 成功, %d 失败"), *ResultMessage, SuccessCount, Results.Num() - SuccessCount)));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoMappingsDefined", "未定义任何映射关系，请先在设置中配置DataTable与Excel的映射"));
	}
}

void FLomoLibEditorModule::OpenSettings()
{
	// 修改为使用GetCategoryName和GetSectionName返回的值
	FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer(
		"Project", 
		"Plugins", 
		"Excel DataTable Converter");
}

// 添加测试Excel转换功能的函数
void FLomoLibEditorModule::TestExcelConverter()
{
	UE_LOG(LogTemp, Log, TEXT("测试Excel转换功能"));
	
	// 获取Python桥接
	auto PythonBridge = UPythonBridge::Get();
	if (!PythonBridge)
	{
		UE_LOG(LogTemp, Error, TEXT("无法获取Python桥接接口"));
		return;
	}
	
	// 测试Excel到CSV的转换
	FString TestExcelPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("TestExcel.xlsx"));
	FString CsvContent = PythonBridge->ExcelToCsv(TestExcelPath);
	
	if (!CsvContent.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("Excel转CSV成功，内容：%s"), *CsvContent.Left(100));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Excel转CSV失败"));
	}
	
	// 测试CSV到Excel的转换
	FString TestCsv = TEXT("Name,Value\nRow1,100\nRow2,200");
	FString OutputExcelPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("TestOutput.xlsx"));
	
	bool bSuccess = PythonBridge->CsvToExcel(TestCsv, OutputExcelPath, TEXT("Sheet1"));
	
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("CSV转Excel成功，保存到：%s"), *OutputExcelPath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CSV转Excel失败"));
	}
}

// 添加新方法，使用内容浏览器获取选中的DataTable
UDataTable* FLomoLibEditorModule::GetSelectedDataTable()
{
	TArray<FAssetData> SelectedAssets;
	
	// 获取内容浏览器模块
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	IContentBrowserSingleton& ContentBrowser = ContentBrowserModule.Get();
	
	// 获取内容浏览器中选中的资产
	ContentBrowser.GetSelectedAssets(SelectedAssets);
	
	// 查找DataTable类型的资产
	for (const FAssetData& AssetData : SelectedAssets)
	{
		if (AssetData.AssetClassPath == UDataTable::StaticClass()->GetClassPathName())
		{
			return Cast<UDataTable>(AssetData.GetAsset());
		}
	}
	
	return nullptr;
}

void FLomoLibEditorModule::RefreshAnimationAssets()
{
	auto* Settings = GetMutableDefault<UProgramAnimationSettings>();
	
	// 清空现有的程序动画资产映射
	Settings->ProgramAnimationAssets.Empty();
	
	// 构建程序动画数据资产搜索路径
	int32 FoundAnimationCount = 0;
	
	if (Settings->ProgramAnimationAssetRootDir.Num() > 0)
	{
		for (const FString& RootDir : Settings->ProgramAnimationAssetRootDir)
		{
			if (!RootDir.IsEmpty())
			{
				// 🔧 使用多个路径前缀来查询
				TArray<FString> SearchPrefixes = {
					FPaths::ProjectContentDir(),           // 主项目Content目录
					FPaths::ProjectDir(),                   // 项目根目录
					FPaths::Combine(FPaths::ProjectDir(), TEXT("Plugins"))  // 插件目录
				};
				
				TArray<FString> AnimationAssetPaths;
				
				for (const FString& Prefix : SearchPrefixes)
				{
					FString AnimationSearchPath = FPaths::Combine(Prefix, RootDir);
					
					UE_LOG(LogTemp, Display, TEXT("🔍 尝试搜索路径: %s"), *AnimationSearchPath);
					
					if (FPaths::DirectoryExists(AnimationSearchPath))
					{
						TArray<FString> TempPaths;
						IFileManager::Get().FindFilesRecursive(TempPaths, *AnimationSearchPath, TEXT("*.uasset"), true, false);
						AnimationAssetPaths.Append(TempPaths);
						
						UE_LOG(LogTemp, Display, TEXT("✅ 在路径 %s 找到 %d 个文件"), *AnimationSearchPath, TempPaths.Num());
					}
					else
					{
						UE_LOG(LogTemp, Display, TEXT("❌ 路径不存在: %s"), *AnimationSearchPath);
					}
				}

				for (const FString& AssetPath : AnimationAssetPaths)
				{
					// 🔧 智能转换物理路径为UE资源路径
					FString RelativePath = AssetPath;
					FString UEPath;
					
					// 移除 .uasset 扩展名
					RelativePath.RemoveFromEnd(TEXT(".uasset"));
					
					// 判断路径类型并转换
					if (RelativePath.Contains(TEXT("Plugins")))
					{
						// 插件路径：Plugins/PluginName/Content/... -> /PluginName/...
						int32 PluginIndex = RelativePath.Find(TEXT("Plugins"));
						if (PluginIndex != INDEX_NONE)
						{
							FString PluginPath = RelativePath.Mid(PluginIndex + 8); // 跳过 "Plugins/"
							
							int32 SlashIndex = PluginPath.Find(TEXT("/"));
							if (SlashIndex != INDEX_NONE)
							{
								FString PluginName = PluginPath.Left(SlashIndex);
								FString ContentPath = PluginPath.Mid(SlashIndex + 1);
								
								// 移除 Content/ 前缀
								if (ContentPath.StartsWith(TEXT("Content/")))
								{
									ContentPath = ContentPath.Mid(8); // 跳过 "Content/"
								}
								
								UEPath = FString::Printf(TEXT("/%s/%s"), *PluginName, *ContentPath);
							}
						}
					}
					else
					{
						// 主项目路径：Content/... -> /Game/...
						RelativePath.RemoveFromStart(FPaths::ProjectContentDir());
						UEPath = FString::Printf(TEXT("/Game/%s"), *RelativePath);
					}
					
					// 路径格式统一化
					UEPath.ReplaceInline(TEXT("\\"), TEXT("/"));
					
					UE_LOG(LogTemp, Display, TEXT("🎯 转换路径: %s -> %s"), *AssetPath, *UEPath);

					// 尝试加载资产
					UObject* LoadedAsset = LoadObject<UObject>(nullptr, *UEPath);
					UProgramAnimationDataAsset* AnimationAsset = Cast<UProgramAnimationDataAsset>(LoadedAsset);
					
					if (AnimationAsset)
					{
						// 使用资产名称作为Key，软引用作为Value
						TSoftObjectPtr<UProgramAnimationDataAsset> SoftRef = TSoftObjectPtr<UProgramAnimationDataAsset>(AnimationAsset);
						
						Settings->ProgramAnimationAssets.Add(AnimationAsset->AnimationData.AnimationName, SoftRef);
						FoundAnimationCount++;
						
						UE_LOG(LogTemp, Display, TEXT("找到程序动画数据资产: %s (从路径: %s)"), *AnimationAsset->GetName(), *RootDir);
					}
				}
			}
		}
	}
	
	// 保存设置
	Settings->MarkPackageDirty();
	Settings->SaveConfig();
	Settings->TryUpdateDefaultConfigFile();
	
	// 显示结果
	FString ResultMessage = FString::Printf(TEXT("动画资产刷新完成！\n找到 %d 个动画资产。"), FoundAnimationCount);
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ResultMessage));
}

void FLomoLibEditorModule::GenerateReverseAnimations()
{
	auto* Settings = GetMutableDefault<UProgramAnimationSettings>();
	Settings->GeneratedReverseAnimations.Empty();
	
	int32 GeneratedCount = 0;
	
	// 遍历需要反转的动画
	for (const FName& AnimationName : Settings->NeedReverseProgramAnimations)
	{
		if (Settings->ProgramAnimationAssets.Contains(AnimationName))
		{
			auto AnimAsset = Settings->ProgramAnimationAssets[AnimationName].LoadSynchronous();
			if (AnimAsset)
			{
				// 调用静态方法生成反转动画
				FProgramAnimationData ReverseData = 
					FProgramAnimationData::GenerateReverseAnimation(AnimAsset->AnimationData);
				
				Settings->GeneratedReverseAnimations.Add(ReverseData.AnimationName, ReverseData);
				GeneratedCount++;
				
				UE_LOG(LogTemp, Log, TEXT("Generated reverse animation: %s"), 
					*ReverseData.AnimationName.ToString());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to generate reverse animation: %s not found"), 
				*AnimationName.ToString());
		}
	}
	
	// 保存设置
	Settings->MarkPackageDirty();
	Settings->SaveConfig();
	Settings->TryUpdateDefaultConfigFile();
	
	// 显示结果
	FString ResultMessage = FString::Printf(TEXT("反转动画生成完成！\n生成了 %d 个反转动画数据。"), GeneratedCount);
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ResultMessage));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLomoLibEditorModule, LomoLibEditor)
