// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LomoLibBlueprintFunctionLibrary.generated.h"
USTRUCT(BlueprintType)
struct LOMOLIB_API FFaceRotation
{
	GENERATED_BODY()

	FFaceRotation(): FaceRotation()
	{
	}

	FFaceRotation(bool bInNeedRotate, const FRotator& InFaceRotation)
		: bNeedRotate(bInNeedRotate), FaceRotation(InFaceRotation)
	{
	}

	UPROPERTY(BlueprintReadOnly)
	bool bNeedRotate = true;

	UPROPERTY(BlueprintReadOnly, Category = "EXGAS|FaceRotation")
	FRotator FaceRotation;
};

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
	// For WaitGroup
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static void DoneWaitGroupOnce(UObject* WorldContextObject, const int32 InWaitGroupID, const FName InDebugText = TEXT(""));

	/**
	 * 从InSourceLocation面向InTargetLocation的方向
	 * @param InTargetLocation
	 * @param InSourceLocation 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable)
	static FFaceRotation GetFaceRotation(const FVector& InTargetLocation, const FVector& InSourceLocation);
};
