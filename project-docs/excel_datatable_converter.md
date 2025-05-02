# Excel与DataTable转换功能

本文档描述了如何使用Python桥接方式实现Excel和DataTable之间的转换功能。

## 功能概述

该功能使用Python的pandas库来处理Excel文件，实现了以下功能：

1. **DataTable导出到Excel**：将UE中的DataTable数据导出为Excel文件
2. **Excel导入到DataTable**：将Excel文件中的数据导入到UE的DataTable中
3. **批量导出到多个Sheet**：支持将多个DataTable导出到同一个Excel文件的不同Sheet中

## 实现方式

整个转换过程分为两个部分：

### DataTable到Excel的转换流程

1. 在C++中将DataTable转换为CSV格式的字符串
2. 使用UPythonBridge调用Python脚本，将CSV字符串转换为Excel文件(.xlsx)
3. 当导出到同一Excel文件时，会保留其他Sheet中的内容，只替换或创建指定的Sheet

### Excel到DataTable的转换流程

1. 使用UPythonBridge调用Python脚本，将Excel文件转换为CSV格式的字符串
2. 在C++中，将CSV字符串解析并写入到DataTable

## 环境要求

要使用此功能，需要安装以下Python库：

- pandas
- openpyxl（用于Excel文件处理）
- pypinyin（已有功能的依赖）

可以运行`CheckLibraries.py`脚本来检查和安装这些库。

## 使用方法

### 在编辑器中使用

1. 在内容浏览器中选择DataTable资产
2. 点击右键菜单，选择"Excel DataTable 转换器"
3. 选择"导入Excel"或"导出Excel"

### 批量导出到多个Sheet

1. 打开设置（Edit -> Project Settings -> Plugins -> Excel DataTable 转换器）
2. 添加多个映射，指向同一个Excel文件，但使用不同的Sheet名称
3. 使用"批量导出"功能，将多个DataTable导出到同一个Excel文件的不同Sheet中

### 通过代码使用

```cpp
// 获取转换器实例
UExcelDataTableConverter* Converter = GEditor->GetEditorSubsystem<UExcelDataTableConverter>();

// DataTable导出到Excel
FExcelOperationResult ExportResult = Converter->ExportDataTableToExcel(
    TEXT("/Game/Data/MyDataTable"),  // DataTable路径
    TEXT("D:/MyExcelFile.xlsx"),     // Excel文件路径
    TEXT("Sheet1")                   // 工作表名称
);

// Excel导入到DataTable
FExcelOperationResult ImportResult = Converter->ImportExcelToDataTable(
    TEXT("/Game/Data/MyDataTable"),  // DataTable路径
    TEXT("D:/MyExcelFile.xlsx"),     // Excel文件路径
    TEXT("Sheet1")                   // 工作表名称
);
```

## 注意事项

1. 确保Python环境已正确配置，并且所需的库已安装
2. Excel文件的第一行必须是表头，与DataTable的属性名称一致
3. Excel文件的第一列必须是行名（Name）
4. 导入/导出操作可能会导致数据类型转换的问题，尤其是复杂类型如结构体等
5. 批量导出到同一Excel文件的不同Sheet时，对每个Sheet单独操作，不会影响其他Sheet的内容

## 故障排除

如果遇到转换问题，请检查以下几点：

1. 运行`CheckLibraries.py`确认所需Python库已正确安装
2. 检查Excel文件格式是否正确（表头、数据类型等）
3. 检查UE日志，查看是否有详细的错误信息
4. 确保Python桥接接口正常工作（可以通过调用`PythonBridge->Test()`测试） 