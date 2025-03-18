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
};
