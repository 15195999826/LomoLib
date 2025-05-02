// Fill out your copyright notice in the Description page of Project Settings.

#include "ExcelDataTable/ExcelDataTableConverter.h"
#include "ExcelDataTable/ExcelDataTableSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/DataTable.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "PropertyEditorModule.h"
#include "UObject/PropertyPortFlags.h"
#include "UObject/TextProperty.h"
#include "PythonBridge.h"

void UExcelDataTableConverter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UExcelDataTableConverter::Deinitialize()
{
	Super::Deinitialize();
}

FExcelOperationResult UExcelDataTableConverter::ImportExcelToDataTable(const FString& DataTablePath, const FString& ExcelFilePath, const FString& SheetName)
{
	FExcelOperationResult Result;
	Result.bSuccess = false;
	
	// 检查文件是否存在
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*ExcelFilePath))
	{
		Result.Message = FString::Printf(TEXT("Excel文件不存在: %s"), *ExcelFilePath);
		return Result;
	}

	// 加载DataTable资产
	UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *DataTablePath);
	if (!DataTable)
	{
		Result.Message = FString::Printf(TEXT("无法加载DataTable: %s"), *DataTablePath);
		return Result;
	}

	// 获取DataTable的行结构
	UScriptStruct* RowStruct = DataTable->RowStruct;
	if (!RowStruct)
	{
		Result.Message = TEXT("DataTable没有有效的行结构");
		return Result;
	}

	// 使用Python桥接将Excel转换为CSV
	auto PythonBridge = UPythonBridge::Get();
	if (!PythonBridge)
	{
		Result.Message = TEXT("无法获取Python桥接接口");
		return Result;
	}
	
	// 调用Python脚本将Excel转换为CSV
	FString CsvContent = PythonBridge->ExcelToCsv(ExcelFilePath);
	if (CsvContent.IsEmpty())
	{
		Result.Message = TEXT("Excel转换为CSV失败，请检查Python环境和pandas库是否已安装");
		return Result;
	}
	
	// 解析CSV内容
	TArray<TArray<FString>> Rows;
	TArray<FString> Lines;
	CsvContent.ParseIntoArrayLines(Lines, false);
	
	for (const FString& Line : Lines)
	{
		TArray<FString> Values;
		Line.ParseIntoArray(Values, TEXT(","), true);
		Rows.Add(Values);
	}
	
	if (Rows.Num() < 3)  // 现在需要至少有表头、类型行和一行数据
	{
		Result.Message = TEXT("CSV内容不足，至少需要表头、类型行和一行数据");
		return Result;
	}
	
	// 第一行是表头
	TArray<FString> Headers = Rows[0];
	
	// 第二行是类型信息，我们可以跳过它
	
	// 把第一列表头从"Row_Name"转换回"Name"
	if (Headers.Num() > 0 && Headers[0] == TEXT("Row_Name"))
	{
		Headers[0] = TEXT("Name");
	}
	
	// 清空现有数据
	DataTable->EmptyTable();
	
	// 从第三行开始处理数据
	for (int32 RowIndex = 2; RowIndex < Rows.Num(); RowIndex++)
	{
		const TArray<FString>& RowData = Rows[RowIndex];
		
		// 检查第一列是否为有效的行名
		if (RowData.Num() == 0 || RowData[0].IsEmpty())
		{
			Result.ErrorRows.Add(RowIndex);
			continue;
		}
		
		FName RowName(*RowData[0]);
		
		// 创建新行
		uint8* NewRowData = (uint8*)FMemory::Malloc(RowStruct->GetStructureSize());
		RowStruct->InitializeStruct(NewRowData);
		
		// 填充行数据
		bool bRowValid = true;
		for (int32 ColIndex = 1; ColIndex < FMath::Min(Headers.Num(), RowData.Num()); ColIndex++)
		{
			FString PropertyName = Headers[ColIndex];
			FString PropertyValue = RowData[ColIndex];
			
			FProperty* Property = RowStruct->FindPropertyByName(*PropertyName);
			if (Property)
			{
				// 设置属性值
				void* PropertyAddr = Property->ContainerPtrToValuePtr<void>(NewRowData);
				
				// 使用FProperty的ImportText_Direct方法，注意参数类型
				const TCHAR* ErrorText = Property->ImportText_Direct(*PropertyValue, PropertyAddr, nullptr, PPF_None, nullptr);
				if (ErrorText != nullptr)
				{
					UE_LOG(LogTemp, Error, TEXT("导入失败: %s"), ErrorText);
					bRowValid = false;
					break;
				}
			}
		}
		
		if (bRowValid)
		{
			// 添加到DataTable
			DataTable->AddRow(RowName, *(FTableRowBase*)NewRowData);
			Result.ProcessedRows++;
		}
		else
		{
			Result.ErrorRows.Add(RowIndex);
		}
		
		// 清理内存
		RowStruct->DestroyStruct(NewRowData);
		FMemory::Free(NewRowData);
	}
	
	// 标记DataTable为已修改
	DataTable->MarkPackageDirty();
	
	Result.bSuccess = Result.ProcessedRows > 0;
	Result.Message = FString::Printf(TEXT("成功导入 %d 行数据，%d 行数据出错"), Result.ProcessedRows, Result.ErrorRows.Num());
	
	return Result;
}

FExcelOperationResult UExcelDataTableConverter::ExportDataTableToExcel(const FString& DataTablePath, const FString& ExcelFilePath, const FString& SheetName)
{
	FExcelOperationResult Result;
	Result.bSuccess = false;
	
	// 加载DataTable资产
	UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *DataTablePath);
	if (!DataTable)
	{
		Result.Message = FString::Printf(TEXT("无法加载DataTable: %s"), *DataTablePath);
		return Result;
	}

	// 获取DataTable的行结构
	UScriptStruct* RowStruct = DataTable->RowStruct;
	if (!RowStruct)
	{
		Result.Message = TEXT("DataTable没有有效的行结构");
		return Result;
	}

	// 创建导出的行数据，先转换为CSV格式
	TArray<TArray<FString>> ExportRows;
	
	// 添加表头
	TArray<FString> Headers = CreateHeaderFromRowStruct(RowStruct);
	
	// 将第一列的"Name"改为"Row_Name"
	if (Headers.Num() > 0 && Headers[0] == TEXT("Name"))
	{
		Headers[0] = TEXT("Row_Name");
	}
	
	ExportRows.Add(Headers);
	
	// 添加类型行
	TArray<FString> TypeRow = CreateTypeRowFromRowStruct(RowStruct);
	ExportRows.Add(TypeRow);
	
	// 遍历DataTable中的所有行
	for (auto& RowPair : DataTable->GetRowMap())
	{
		FName RowName = RowPair.Key;
		uint8* RowData = (uint8*)RowPair.Value;
		
		TArray<FString> RowValues;
		RowValues.Add(RowName.ToString()); // 第一列是行名
		
		// 获取每个属性的值
		for (int32 i = 1; i < Headers.Num(); i++)
		{
			FString PropertyName;
			
			// 针对第一列表头的特殊处理
			if (i == 0 && Headers[i] == TEXT("Row_Name"))
			{
				PropertyName = TEXT("Name");
			}
			else
			{
				PropertyName = Headers[i];
			}
			
			FProperty* Property = RowStruct->FindPropertyByName(*PropertyName);
			
			if (Property)
			{
				void* PropertyAddr = Property->ContainerPtrToValuePtr<void>(RowData);
				
				// 使用Property的ExportTextItem_Direct方法
				FString PropertyValue;
				Property->ExportTextItem_Direct(PropertyValue, PropertyAddr, nullptr, nullptr, PPF_None);
				RowValues.Add(PropertyValue);
			}
			else
			{
				RowValues.Add(TEXT(""));
			}
		}
		
		ExportRows.Add(RowValues);
		Result.ProcessedRows++;
	}
	
	// 将行数据转换为CSV字符串，使用UTF-8编码
	TArray<FString> Lines;
	for (const TArray<FString>& Row : ExportRows)
	{
		TArray<FString> EscapedValues;
		for (const FString& Value : Row)
		{
			// 如果值包含逗号或引号，需要用双引号包围
			FString EscapedValue = Value;
			
			// 替换所有引号为两个引号，确保转义正确
			if (EscapedValue.Contains(TEXT("\"")))
			{
				EscapedValue.ReplaceInline(TEXT("\""), TEXT("\"\""));
			}
			
			// 所有值都用引号包围，以确保特殊字符和Unicode字符正确处理
			EscapedValue = FString::Printf(TEXT("\"%s\""), *EscapedValue);
			
			EscapedValues.Add(EscapedValue);
		}
		
		Lines.Add(FString::Join(EscapedValues, TEXT(",")));
	}
	
	FString CsvContent = FString::Join(Lines, TEXT("\n"));
	
	// 使用Python桥接将CSV转换为Excel
	auto PythonBridge = UPythonBridge::Get();
	if (!PythonBridge)
	{
		Result.Message = TEXT("无法获取Python桥接接口");
		return Result;
	}
	
	// 调用Python脚本将CSV转换为Excel
	bool bSuccess = PythonBridge->CsvToExcel(CsvContent, ExcelFilePath, SheetName);
	if (bSuccess)
	{
		Result.bSuccess = true;
		Result.Message = FString::Printf(TEXT("成功导出 %d 行数据到 %s"), Result.ProcessedRows, *ExcelFilePath);
	}
	else
	{
		Result.Message = FString::Printf(TEXT("无法将CSV转换为Excel，请检查Python环境和pandas库是否已安装"));
	}
	
	return Result;
}

TArray<FExcelOperationResult> UExcelDataTableConverter::BatchProcess(bool bImport)
{
	TArray<FExcelOperationResult> Results;
	
	// 获取设置
	UExcelDataTableSettings* Settings = GetMutableDefault<UExcelDataTableSettings>();
	if (!Settings)
	{
		FExcelOperationResult Result;
		Result.bSuccess = false;
		Result.Message = TEXT("无法加载ExcelDataTableSettings");
		Results.Add(Result);
		return Results;
	}
	
	// 处理每个映射
	for (const FExcelDataTableMapping& Mapping : Settings->DataTableMappings)
	{
		FString DataTablePath = Mapping.DataTablePath.ToString();
		FString ExcelFilePath = Mapping.ExcelFilePath.FilePath;
		FString SheetName = Mapping.SheetName;
		
		if (DataTablePath.IsEmpty() || ExcelFilePath.IsEmpty())
		{
			continue;
		}
		
		FExcelOperationResult Result;
		if (bImport)
		{
			Result = ImportExcelToDataTable(DataTablePath, ExcelFilePath, SheetName);
		}
		else
		{
			Result = ExportDataTableToExcel(DataTablePath, ExcelFilePath, SheetName);
		}
		
		Results.Add(Result);
	}
	
	return Results;
}

bool UExcelDataTableConverter::ReadCSVFile(const FString& FilePath, TArray<TArray<FString>>& OutRows)
{
	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
	{
		return false;
	}
	
	TArray<FString> Lines;
	FileContent.ParseIntoArrayLines(Lines, false);
	
	for (const FString& Line : Lines)
	{
		TArray<FString> Values;
		Line.ParseIntoArray(Values, TEXT(","), true);
		OutRows.Add(Values);
	}
	
	return true;
}

bool UExcelDataTableConverter::WriteCSVFile(const FString& FilePath, const TArray<TArray<FString>>& Rows)
{
	TArray<FString> Lines;
	
	for (const TArray<FString>& Row : Rows)
	{
		TArray<FString> EscapedValues;
		for (const FString& Value : Row)
		{
			// 如果值包含逗号或引号，需要用双引号包围
			FString EscapedValue = Value;
			bool bNeedsQuotes = Value.Contains(TEXT(",")) || Value.Contains(TEXT("\""));
			
			if (bNeedsQuotes)
			{
				// 替换所有引号为两个引号
				EscapedValue.ReplaceInline(TEXT("\""), TEXT("\"\""));
				EscapedValue = FString::Printf(TEXT("\"%s\""), *EscapedValue);
			}
			
			EscapedValues.Add(EscapedValue);
		}
		
		Lines.Add(FString::Join(EscapedValues, TEXT(",")));
	}
	
	return FFileHelper::SaveStringArrayToFile(Lines, *FilePath);
}

TArray<FString> UExcelDataTableConverter::CreateHeaderFromRowStruct(UScriptStruct* RowStruct)
{
	TArray<FString> Headers;
	Headers.Add(TEXT("Name")); // 第一列是行名
	
	for (TFieldIterator<FProperty> PropIt(RowStruct); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;
		if (Property->HasAnyPropertyFlags(CPF_Edit) || Property->HasAnyPropertyFlags(CPF_BlueprintVisible))
		{
			// 添加属性名到表头
			FString PropertyName = Property->GetName();
			Headers.Add(PropertyName);
		}
	}
	
	return Headers;
}

TArray<FString> UExcelDataTableConverter::CreateTypeRowFromRowStruct(UScriptStruct* RowStruct)
{
	TArray<FString> TypeRow;
	TypeRow.Add(TEXT("string")); // 第一列是行名，类型为string
	
	for (TFieldIterator<FProperty> PropIt(RowStruct); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;
		if (Property->HasAnyPropertyFlags(CPF_Edit) || Property->HasAnyPropertyFlags(CPF_BlueprintVisible))
		{
			// 获取属性类型
			FString TypeName = GetPropertyTypeName(Property);
			TypeRow.Add(TypeName);
		}
	}
	
	return TypeRow;
}

FString UExcelDataTableConverter::GetPropertyTypeName(FProperty* Property)
{
	if (!Property)
	{
		return TEXT("unknown");
	}
	
	// 根据属性类型返回对应的类型名
	if (Property->IsA<FBoolProperty>())
	{
		return TEXT("bool");
	}
	else if (Property->IsA<FIntProperty>())
	{
		return TEXT("int");
	}
	else if (Property->IsA<FFloatProperty>())
	{
		return TEXT("float");
	}
	else if (Property->IsA<FDoubleProperty>())
	{
		return TEXT("double");
	}
	else if (Property->IsA<FNameProperty>())
	{
		return TEXT("name");
	}
	else if (Property->IsA<FStrProperty>())
	{
		return TEXT("string");
	}
	else if (Property->IsA<FTextProperty>())
	{
		return TEXT("text");
	}
	else if (Property->IsA<FArrayProperty>())
	{
		// 获取数组元素的类型
		FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property);
		if (ArrayProperty && ArrayProperty->Inner)
		{
			FString InnerTypeName = GetPropertyTypeName(ArrayProperty->Inner);
			return FString::Printf(TEXT("array<%s>"), *InnerTypeName);
		}
		return TEXT("array");
	}
	else if (Property->IsA<FMapProperty>())
	{
		// 获取映射的键和值的类型
		FMapProperty* MapProperty = CastField<FMapProperty>(Property);
		if (MapProperty && MapProperty->KeyProp && MapProperty->ValueProp)
		{
			FString KeyTypeName = GetPropertyTypeName(MapProperty->KeyProp);
			FString ValueTypeName = GetPropertyTypeName(MapProperty->ValueProp);
			return FString::Printf(TEXT("map<%s,%s>"), *KeyTypeName, *ValueTypeName);
		}
		return TEXT("map");
	}
	else if (Property->IsA<FSetProperty>())
	{
		// 获取集合元素的类型
		FSetProperty* SetProperty = CastField<FSetProperty>(Property);
		if (SetProperty && SetProperty->ElementProp)
		{
			FString ElementTypeName = GetPropertyTypeName(SetProperty->ElementProp);
			return FString::Printf(TEXT("set<%s>"), *ElementTypeName);
		}
		return TEXT("set");
	}
	else if (Property->IsA<FStructProperty>())
	{
		FStructProperty* StructProperty = CastField<FStructProperty>(Property);
		if (StructProperty && StructProperty->Struct)
		{
			return FString::Printf(TEXT("struct:%s"), *StructProperty->Struct->GetName());
		}
		return TEXT("struct");
	}
	else if (Property->IsA<FEnumProperty>())
	{
		FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property);
		if (EnumProperty && EnumProperty->GetEnum())
		{
			return FString::Printf(TEXT("enum:%s"), *EnumProperty->GetEnum()->GetName());
		}
		return TEXT("enum");
	}
	else if (Property->IsA<FByteProperty>())
	{
		FByteProperty* ByteProperty = CastField<FByteProperty>(Property);
		if (ByteProperty && ByteProperty->Enum)
		{
			return FString::Printf(TEXT("enum:%s"), *ByteProperty->Enum->GetName());
		}
		return TEXT("byte");
	}
	else if (Property->IsA<FObjectProperty>())
	{
		FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property);
		if (ObjectProperty && ObjectProperty->PropertyClass)
		{
			return FString::Printf(TEXT("object:%s"), *ObjectProperty->PropertyClass->GetName());
		}
		return TEXT("object");
	}
	else if (Property->IsA<FSoftObjectProperty>())
	{
		FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(Property);
		if (SoftObjectProperty && SoftObjectProperty->PropertyClass)
		{
			return FString::Printf(TEXT("softobject:%s"), *SoftObjectProperty->PropertyClass->GetName());
		}
		return TEXT("softobject");
	}
	else if (Property->IsA<FSoftClassProperty>())
	{
		FSoftClassProperty* SoftClassProperty = CastField<FSoftClassProperty>(Property);
		if (SoftClassProperty && SoftClassProperty->MetaClass)
		{
			return FString::Printf(TEXT("softclass:%s"), *SoftClassProperty->MetaClass->GetName());
		}
		return TEXT("softclass");
	}
	else if (Property->IsA<FClassProperty>())
	{
		FClassProperty* ClassProperty = CastField<FClassProperty>(Property);
		if (ClassProperty && ClassProperty->MetaClass)
		{
			return FString::Printf(TEXT("class:%s"), *ClassProperty->MetaClass->GetName());
		}
		return TEXT("class");
	}
	
	// 其他类型
	return FString::Printf(TEXT("other:%s"), *Property->GetClass()->GetName());
}

uint8* UExcelDataTableConverter::CreateRowFromStringValues(UScriptStruct* RowStruct, const TArray<FString>& Values, FExcelOperationResult& Result)
{
	uint8* RowData = (uint8*)FMemory::Malloc(RowStruct->GetStructureSize());
	RowStruct->InitializeStruct(RowData);
	
	int32 ValueIndex = 1; // 跳过第一列的行名
	
	for (TFieldIterator<FProperty> PropIt(RowStruct); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;
		if ((Property->HasAnyPropertyFlags(CPF_Edit) || Property->HasAnyPropertyFlags(CPF_BlueprintVisible)) && ValueIndex < Values.Num())
		{
			void* PropertyAddr = Property->ContainerPtrToValuePtr<void>(RowData);
			
			// 使用Property的ImportText_Direct方法，注意参数类型
			const TCHAR* ErrorText = Property->ImportText_Direct(*Values[ValueIndex], PropertyAddr, nullptr, PPF_None, nullptr);
			if (ErrorText != nullptr)
			{
				// 设置属性值失败
				UE_LOG(LogTemp, Error, TEXT("导入失败: %s"), ErrorText);
				RowStruct->DestroyStruct(RowData);
				FMemory::Free(RowData);
				return nullptr;
			}
			
			ValueIndex++;
		}
	}
	
	return RowData;
}

TArray<FString> UExcelDataTableConverter::CreateStringValuesFromRow(UScriptStruct* RowStruct, uint8* RowData)
{
	TArray<FString> Values;
	
	for (TFieldIterator<FProperty> PropIt(RowStruct); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;
		if (Property->HasAnyPropertyFlags(CPF_Edit) || Property->HasAnyPropertyFlags(CPF_BlueprintVisible))
		{
			void* PropertyAddr = Property->ContainerPtrToValuePtr<void>(RowData);
			
			// 使用Property的ExportTextItem_Direct方法
			FString PropertyValue;
			Property->ExportTextItem_Direct(PropertyValue, PropertyAddr, nullptr, nullptr, PPF_None);
			Values.Add(PropertyValue);
		}
	}
	
	return Values;
} 