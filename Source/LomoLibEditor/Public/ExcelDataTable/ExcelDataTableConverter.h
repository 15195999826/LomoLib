// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ExcelDataTableConverter.generated.h"

/**
 * 定义Excel操作的结果
 */
USTRUCT(BlueprintType)
struct FExcelOperationResult
{
	GENERATED_BODY()

	/** 操作是否成功 */
	UPROPERTY(BlueprintReadOnly, Category = "Excel Operation")
	bool bSuccess = false;

	/** 操作结果信息 */
	UPROPERTY(BlueprintReadOnly, Category = "Excel Operation")
	FString Message;

	/** 错误处理的行数 */
	UPROPERTY(BlueprintReadOnly, Category = "Excel Operation")
	TArray<int32> ErrorRows;

	/** 成功转换的数据行数 */
	UPROPERTY(BlueprintReadOnly, Category = "Excel Operation")
	int32 ProcessedRows = 0;
};

/**
 * DataTable与Excel之间转换的功能实现
 */
UCLASS()
class LOMOLIBEDITOR_API UExcelDataTableConverter : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * 将Excel文件导入到DataTable, Todo: 该函数存在功能错误，暂不研究
	 * @param DataTablePath - DataTable资源的路径
	 * @param ExcelFilePath - Excel文件的绝对路径
	 * @param SheetName - 要导入的工作表名称
	 * @return 操作结果
	 */
	UFUNCTION(BlueprintCallable, Category = "Excel DataTable Converter")
	FExcelOperationResult ImportExcelToDataTable(const FString& DataTablePath, const FString& ExcelFilePath, const FString& SheetName = TEXT("Sheet1"));

	/**
	 * 将DataTable导出到Excel文件
	 * @param DataTablePath - DataTable资源的路径
	 * @param ExcelFilePath - Excel文件的绝对路径
	 * @param SheetName - 要导出到的工作表名称
	 * @return 操作结果
	 */
	UFUNCTION(BlueprintCallable, Category = "Excel DataTable Converter")
	FExcelOperationResult ExportDataTableToExcel(const FString& DataTablePath, const FString& ExcelFilePath, const FString& SheetName = TEXT("Sheet1"));

	/**
	 * 批量处理DataTable与Excel之间的转换
	 * @param bImport - 如果为true，则从Excel导入到DataTable；否则从DataTable导出到Excel
	 * @return 操作结果
	 */
	UFUNCTION(BlueprintCallable, Category = "Excel DataTable Converter")
	TArray<FExcelOperationResult> BatchProcess(bool bImport = true);

private:
	/** 读取CSV文件的内容 */
	bool ReadCSVFile(const FString& FilePath, TArray<TArray<FString>>& OutRows);

	/** 写入CSV文件 */
	bool WriteCSVFile(const FString& FilePath, const TArray<TArray<FString>>& Rows);

	/** 根据DataTable的RowStruct创建表头 */
	TArray<FString> CreateHeaderFromRowStruct(UScriptStruct* RowStruct);
	
	/** 根据DataTable的RowStruct创建类型行 */
	TArray<FString> CreateTypeRowFromRowStruct(UScriptStruct* RowStruct);
	
	/** 获取属性的类型名称 */
	FString GetPropertyTypeName(FProperty* Property);

	/** 将数据行转换为DataTable的行 */
	uint8* CreateRowFromStringValues(UScriptStruct* RowStruct, const TArray<FString>& Values, FExcelOperationResult& Result);

	/** 将DataTable的行转换为字符串数组 */
	TArray<FString> CreateStringValuesFromRow(UScriptStruct* RowStruct, uint8* RowData);
}; 