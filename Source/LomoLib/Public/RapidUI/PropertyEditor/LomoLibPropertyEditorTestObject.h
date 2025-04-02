// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LomoLibPropertyEditorTestObject.generated.h"

/**
 * 用于测试嵌套结构体
 */
USTRUCT(BlueprintType)
struct FNestedTestStruct
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test", meta = (DisplayName = "嵌套浮点值"))
    float NestedFloat = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test", meta = (DisplayName = "嵌套字符串"))
    FString NestedString = TEXT("嵌套结构体中的字符串");
};

/**
 * 测试结构体
 */
USTRUCT(BlueprintType)
struct FTestStruct
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test", meta = (DisplayName = "结构体内整数"))
    int32 IntValue = 42;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test", meta = (DisplayName = "结构体内布尔值"))
    bool BoolValue = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test", meta = (DisplayName = "嵌套结构体"))
    FNestedTestStruct NestedStruct;
};

/**
 * 测试对象，用于演示RapidPropertyEditor的功能
 */
UCLASS(BlueprintType)
class LOMOLIB_API ULomoLibPropertyEditorTestObject : public UObject
{
    GENERATED_BODY()

public:
    ULomoLibPropertyEditorTestObject();

    // 基本类型
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Types")
    int32 IntValue = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Types", meta = (DisplayName = "浮点值"))
    float FloatValue = 3.14f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Types", meta = (DisplayName = "字符串"))
    FString StringValue = TEXT("测试字符串");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Types", meta = (DisplayName = "布尔值"))
    bool BoolValue = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Types", meta = (DisplayName = "名称"))
    FName NameValue = TEXT("TestName");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Types", meta = (DisplayName = "文本"))
    FText TextValue = FText::FromString(TEXT("这是一段文本"));

    // 复杂类型
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Complex Types", meta = (DisplayName = "整数数组"))
    TArray<int32> IntArray = {1, 2, 3, 4, 5};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Complex Types", meta = (DisplayName = "字符串数组"))
    TArray<FString> StringArray = {TEXT("第一项"), TEXT("第二项"), TEXT("第三项")};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Complex Types", meta = (DisplayName = "整数到字符串映射"))
    TMap<int32, FString> IntToStringMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Complex Types", meta = (DisplayName = "测试结构体"))
    FTestStruct TestStruct;

    // 不可编辑的属性，不会显示在编辑器中
    UPROPERTY()
    int32 HiddenValue = 999;

    // 打印所有测试属性值
    void PrintAllProperties();
}; 