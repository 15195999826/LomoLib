// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LomoLibBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class LOMOLIB_API ULomoLibBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	// 结构体转JSON字符串
	template<typename T>
	static bool ToJson(const T& InStruct, FString& OutJsonString)
	{
		return FJsonObjectConverter::UStructToJsonObjectString(T::StaticStruct(), &InStruct, OutJsonString, 0, 0);
	}

	// For UI
public:
	UFUNCTION(BlueprintCallable, Category="LomoLib|UI")
	static void EnableUINav();
	UFUNCTION(BlueprintCallable, Category="LomoLib|UI")
	static void DisableUINav();
	UFUNCTION(BlueprintCallable, Category="LomoLib|Debug")
	static void PausePIE();
	// 太难了， 找不到将一个位置从世界坐标转换为相对于某个Actor的局部坐标的函数, 情景: 将A位置设置Attach到B， 获取A现在的相对位置
	// 于是写一个函数， 创建一个Actor, 设置到世界坐标, Attach到B,KeepWorldTransform， 然后获取相对位置的方案。
	UFUNCTION(BlueprintCallable, Category="LomoLib|Debug")
	static FVector WorldToRelativeLocation(UObject* WorldContext, const FVector& InWorldLocation, AActor* RelativeToActor);
};
