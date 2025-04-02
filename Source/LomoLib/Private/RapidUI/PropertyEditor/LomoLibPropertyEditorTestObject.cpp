// Fill out your copyright notice in the Description page of Project Settings.

#include "RapidUI/PropertyEditor/LomoLibPropertyEditorTestObject.h"

ULomoLibPropertyEditorTestObject::ULomoLibPropertyEditorTestObject()
{
    // 初始化Map
    IntToStringMap.Add(1, TEXT("一"));
    IntToStringMap.Add(2, TEXT("二"));
    IntToStringMap.Add(3, TEXT("三"));
}

void ULomoLibPropertyEditorTestObject::PrintAllProperties()
{
    // 打印基本类型
    UE_LOG(LogTemp, Log, TEXT("===== 基本类型 ====="));
    UE_LOG(LogTemp, Log, TEXT("IntValue: %d"), IntValue);
    UE_LOG(LogTemp, Log, TEXT("FloatValue: %f"), FloatValue);
    UE_LOG(LogTemp, Log, TEXT("StringValue: %s"), *StringValue);
    UE_LOG(LogTemp, Log, TEXT("BoolValue: %s"), BoolValue ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Log, TEXT("NameValue: %s"), *NameValue.ToString());
    UE_LOG(LogTemp, Log, TEXT("TextValue: %s"), *TextValue.ToString());
    
    // 打印复杂类型
    UE_LOG(LogTemp, Log, TEXT("===== 复杂类型 ====="));
    
    // 打印整数数组
    UE_LOG(LogTemp, Log, TEXT("IntArray: [%d 个元素]"), IntArray.Num());
    for (int32 i = 0; i < IntArray.Num(); ++i)
    {
        UE_LOG(LogTemp, Log, TEXT("  [%d]: %d"), i, IntArray[i]);
    }
    
    // 打印字符串数组
    UE_LOG(LogTemp, Log, TEXT("StringArray: [%d 个元素]"), StringArray.Num());
    for (int32 i = 0; i < StringArray.Num(); ++i)
    {
        UE_LOG(LogTemp, Log, TEXT("  [%d]: %s"), i, *StringArray[i]);
    }
    
    // 打印映射
    UE_LOG(LogTemp, Log, TEXT("IntToStringMap: [%d 个元素]"), IntToStringMap.Num());
    for (const auto& Pair : IntToStringMap)
    {
        UE_LOG(LogTemp, Log, TEXT("  [%d]: %s"), Pair.Key, *Pair.Value);
    }
    
    // 打印结构体
    UE_LOG(LogTemp, Log, TEXT("TestStruct:"));
    UE_LOG(LogTemp, Log, TEXT("  IntValue: %d"), TestStruct.IntValue);
    UE_LOG(LogTemp, Log, TEXT("  BoolValue: %s"), TestStruct.BoolValue ? TEXT("true") : TEXT("false"));
    UE_LOG(LogTemp, Log, TEXT("  NestedStruct:"));
    UE_LOG(LogTemp, Log, TEXT("    NestedFloat: %f"), TestStruct.NestedStruct.NestedFloat);
    UE_LOG(LogTemp, Log, TEXT("    NestedString: %s"), *TestStruct.NestedStruct.NestedString);
    
    // 打印不可见属性（仅供完整性）
    UE_LOG(LogTemp, Log, TEXT("===== 不可见属性 ====="));
    UE_LOG(LogTemp, Log, TEXT("HiddenValue: %d"), HiddenValue);
}