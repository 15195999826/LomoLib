// Fill out your copyright notice in the Description page of Project Settings.

#include "ExcelDataTable/ExcelDataTableSettings.h"

UExcelDataTableSettings::UExcelDataTableSettings()
{
}

FName UExcelDataTableSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

FName UExcelDataTableSettings::GetSectionName() const
{
	return TEXT("Excel DataTable Converter");
}

FExcelDataTableMapping* UExcelDataTableSettings::GetMappingForDataTable(const FSoftObjectPath& DataTablePath)
{
	for (FExcelDataTableMapping& Mapping : DataTableMappings)
	{
		if (Mapping.DataTablePath == DataTablePath)
		{
			return &Mapping;
		}
	}
	return nullptr;
}

FExcelDataTableMapping* UExcelDataTableSettings::GetMappingForExcelFile(const FString& ExcelFilePath)
{
	for (FExcelDataTableMapping& Mapping : DataTableMappings)
	{
		if (Mapping.ExcelFilePath.FilePath == ExcelFilePath)
		{
			return &Mapping;
		}
	}
	return nullptr;
}