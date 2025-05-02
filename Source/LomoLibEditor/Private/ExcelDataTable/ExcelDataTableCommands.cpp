#include "ExcelDataTable/ExcelDataTableCommands.h"
#include "Framework/Commands/UIAction.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "ExcelDataTableCommands"

FExcelDataTableCommands::FExcelDataTableCommands()
	: TCommands<FExcelDataTableCommands>(
		TEXT("ExcelDataTable"), // Context name for fast lookup
		NSLOCTEXT("ExcelDataTable", "ExcelDataTableCommands", "Excel DataTable 转换器"), // Context name for displaying
		NAME_None, // Parent
		FAppStyle::GetAppStyleSetName() // Icon Style Set
	)
{
}

void FExcelDataTableCommands::RegisterCommands()
{
	UI_COMMAND(BatchImport, "批量导入", "批量导入Excel文件到DataTable", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(BatchExport, "批量导出", "批量导出DataTable到Excel文件", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenSettings, "打开设置", "打开Excel DataTable转换器设置", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE 