// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ExcelDataTableSettings.generated.h"

/**
 * 定义Excel文件与DataTable之间的映射关系
 */
USTRUCT(BlueprintType)
struct FExcelDataTableMapping
{
	GENERATED_BODY()

	/** DataTable资源路径 */
	UPROPERTY(EditAnywhere, Category = "Mapping", meta = (AllowedClasses = "/Script/Engine.DataTable"))
	FSoftObjectPath DataTablePath;

	/** Excel文件路径 */
	UPROPERTY(EditAnywhere, Category = "Mapping", meta = (FilePathFilter = "Excel Files (*.xlsx;*.xls)|*.xlsx;*.xls"))
	FFilePath ExcelFilePath;

	/** Excel中的工作表名称 */
	UPROPERTY(EditAnywhere, Category = "Mapping")
	FString SheetName = TEXT("Sheet1");

	/** 是否自动同步 */
	UPROPERTY(EditAnywhere, Category = "Mapping")
	bool bAutoSync = false;
};

/**
 * DataTable与Excel转换工具的配置设置
 */
UCLASS(config = Editor, defaultconfig, meta = (DisplayName = "Excel DataTable Converter"))
class LOMOLIBEDITOR_API UExcelDataTableSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UExcelDataTableSettings();

	// 返回这些设置应该分组的类别名称
	virtual FName GetCategoryName() const override;
	// 返回这些设置应该在类别内显示的部分名称
	virtual FName GetSectionName() const override;
	
	/** DataTable与Excel文件的映射 */
	UPROPERTY(config, EditAnywhere, Category = "Mappings", meta = (TitleProperty = "DataTablePath"))
	TArray<FExcelDataTableMapping> DataTableMappings;

	/** 获取与特定DataTable相关联的Excel映射 */
	FExcelDataTableMapping* GetMappingForDataTable(const FSoftObjectPath& DataTablePath);

	/** 获取与特定Excel文件相关联的映射 */
	FExcelDataTableMapping* GetMappingForExcelFile(const FString& ExcelFilePath);
}; 