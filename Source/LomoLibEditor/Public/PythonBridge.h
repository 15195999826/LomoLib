// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PythonBridge.generated.h"

/**
 * 
 */
UCLASS()
class LOMOLIBEDITOR_API UPythonBridge : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = Python)
	static UPythonBridge* Get();

	UFUNCTION(BlueprintImplementableEvent, Category = Python)
	void FunctionImplementedInPython() const;

	UFUNCTION(BlueprintImplementableEvent, Category = Python)
	FString Test() const;

	UFUNCTION(BlueprintImplementableEvent, Category = Python)
	FString ToPinyin(const FString& InText) const;

	// Excel转换为CSV格式
	UFUNCTION(BlueprintImplementableEvent, Category = Python)
	FString ExcelToCsv(const FString& ExcelPath) const;

	// Excel转换为JSON格式
	UFUNCTION(BlueprintImplementableEvent, Category = Python)
	FString ExcelToJson(const FString& ExcelPath, const FString& SheetName) const;

	// CSV格式转换为Excel
	UFUNCTION(BlueprintImplementableEvent, Category = Python)
	bool CsvToExcel(const FString& CsvString, const FString& ExcelPath, const FString& SheetName) const;

	// JSON格式转换为Excel
	UFUNCTION(BlueprintImplementableEvent, Category = Python)
	bool JsonToExcel(const FString& JsonString, const FString& ExcelPath, const FString& SheetName) const;
};
