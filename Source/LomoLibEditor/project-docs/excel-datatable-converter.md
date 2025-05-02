# DataTable与Excel转换工具

## 功能概述

DataTable与Excel转换工具提供了一种简便的方法，可以在Unreal Engine的DataTable与Excel文件之间进行双向转换。该工具允许开发者通过配置文件设置DataTable与Excel文件的关联关系，以便于数据的管理和编辑。

## 核心功能

1. **从Excel导入到DataTable**：将指定Excel文件的数据导入到UE中的DataTable中
2. **从DataTable导出到Excel**：将UE中的DataTable数据导出到Excel文件中
3. **批量转换**：支持批量导入/导出多个DataTable与Excel文件
4. **配置管理**：通过DeveloperSettings提供一个界面，用于配置DataTable与Excel文件的关联关系

## 技术实现

1. **配置系统**：
   - 创建`UExcelDataTableSettings`类，继承自`UDeveloperSettings`
   - 在编辑器中提供配置界面，允许用户设置DataTable与Excel文件的映射关系

2. **转换逻辑**：
   - 使用UE的属性系统进行数据转换，使用`ImportText`导入值，`ExportTextItem`导出值
   - 支持CSV格式的Excel文件读写
   - 自动处理数据类型转换，包括字符串、数字、枚举和结构体等

3. **编辑器集成**：
   - 在编辑器菜单中添加导入/导出选项
   - 提供批量操作功能
   - 支持从内容浏览器中选择DataTable资产

## 属性值转换实现

工具利用UE的属性系统进行数据的导入和导出：

1. **CSV解析**：
   - 解析CSV文件并识别表头（第一行）作为属性名称
   - 使用第一列作为DataTable的行名称

2. **属性值导入**：
   - 使用`FProperty::ImportText`方法将字符串值转换为属性值
   - 支持所有UE可序列化的类型，包括基本类型、枚举和结构体
   - 处理导入错误并提供详细的错误信息

3. **属性值导出**：
   - 使用`FProperty::ExportTextItem`方法将属性值转换为字符串
   - 正确处理特殊字符和引号
   - 支持从DataTable中的任何属性类型导出

## 使用场景

1. **游戏数据管理**：游戏开发者可以在Excel中编辑游戏数据，然后一键导入到游戏中
2. **数据维护**：策划人员可以直接在Excel中修改数据，而不需要学习UE编辑器
3. **团队协作**：便于数据在团队成员之间共享和版本控制

## 如何使用

1. **单个DataTable转换**：
   - 在内容浏览器中选择DataTable资产
   - 使用工具菜单中的导入/导出选项
   - 选择Excel文件并完成转换

2. **批量转换**：
   - 在设置中配置DataTable与Excel文件的映射关系
   - 使用批量导入/导出功能一次性处理多个文件

## 配置示例

```json
{
  "DataTableMappings": [
    {
      "DataTablePath": "/Game/Data/ItemTable",
      "ExcelFilePath": "D:/GameData/Items.xlsx",
      "SheetName": "Items"
    },
    {
      "DataTablePath": "/Game/Data/SkillTable",
      "ExcelFilePath": "D:/GameData/Skills.xlsx",
      "SheetName": "ActiveSkills"
    }
  ]
}
```

## 注意事项

1. Excel文件的第一行必须是列名，对应DataTable的属性名
2. 支持的数据类型包括：int、float、string、bool、枚举、结构体等UE可序列化的类型
3. 复杂数据类型需要确保导入格式正确，例如结构体应使用括号，数组应使用括号和逗号分隔
4. 数据转换过程中可能出现的错误将记录到日志中，并在转换结果中显示 