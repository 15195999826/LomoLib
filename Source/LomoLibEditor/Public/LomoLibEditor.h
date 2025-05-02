// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;
class FExcelDataTableCommands;
class ISettingsSection;
class FUICommandList;
class UDataTable;

/**
 * LomoLibEditor模块类
 */
class FLomoLibEditorModule : public IModuleInterface
{
public:
	// IModuleInterface接口
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	// Excel DataTable转换器
	void RegisterExcelDataTableCommands();
	void UnregisterExcelDataTableCommands();
	void RegisterExcelDataTableSettings();
	void UnregisterExcelDataTableSettings();
	void RegisterExcelDataTableMenu();
	void UnregisterExcelDataTableMenu();

	// 命令处理函数
	void ImportExcelToDataTable();
	void ExportDataTableToExcel();
	void BatchImport();
	void BatchExport();
	void OpenSettings();
	
	// 测试函数
	void TestExcelConverter();
	
	
	// 辅助方法
	UDataTable* GetSelectedDataTable();

private:
	TSharedPtr<FUICommandList> ExcelDataTableCommands;
	TSharedPtr<FExcelDataTableCommands> ExcelDataTableCommandsImpl;
	TSharedPtr<ISettingsSection> ExcelDataTableSettingsSection;
	TSharedPtr<class FExtender> ToolbarExtender;
};
