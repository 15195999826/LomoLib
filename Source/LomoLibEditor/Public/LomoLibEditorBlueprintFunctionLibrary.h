// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/ProgramAnimationTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LomoLibEditorBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class LOMOLIBEDITOR_API ULomoLibEditorBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	// 创建程序动画资产
	UFUNCTION(BlueprintCallable)
	static EProgramAnimationAssetCreateResult CreateProgramAnimationDataAsset(
		const FProgramAnimationData& AnimationData, 
		const FString& AssetPath,
		bool bOverwrite = true
	);
};
