// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/LatentActionManager.h"
#include "ICancellable.h"
#include "Animation/ProgramAnimationTypes.h"
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

	/**
	 * 可以取消的延时函数，取消后， 后续函数不会被调用
	 * @param WorldContextObject The world context object
	 * @param Duration The duration to delay in seconds
	 * @param LatentInfo The latent action info for blueprint execution flow
	 * @param CancellableContext The object that implements ICancellable interface for cancellation checks
	 */
	UFUNCTION(BlueprintCallable, meta = (Latent, WorldContext="WorldContextObject", LatentInfo = "LatentInfo", Category = "LomoLib|Async"))
	static void DelayWithCancel(const UObject* WorldContextObject, float Duration, FLatentActionInfo LatentInfo, TScriptInterface<ICancellable> CancellableContext);

	// Template function for getting user config values of different types
	template<typename T>
	static T GetUserConfig(const FString& InKey, const T& InDefaultValue);

	// Explicit template specializations
	template<>
	float GetUserConfig<float>(const FString& InKey, const float& InDefaultValue);

	template<>
	int32 GetUserConfig<int32>(const FString& InKey, const int32& InDefaultValue);

	template<>
	bool GetUserConfig<bool>(const FString& InKey, const bool& InDefaultValue);

	template<>
	FString GetUserConfig<FString>(const FString& InKey, const FString& InDefaultValue);

	// ====== For ProgramAnimation ======
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static const FProgramAnimationData& GetProgramAnimationData(const FName& InAniName);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (WorldContext = "WorldContextObject"))
	static FProgramAnimationData RotateProgramAnimationData(const FName& InAniName, const FVector& InDirection, const FVector& BaseAnimationDirection = FVector(1,0,0));
};
