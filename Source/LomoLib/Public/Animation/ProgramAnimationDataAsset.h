// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Animation/ProgramAnimationTypes.h"
#include "ProgramAnimationDataAsset.generated.h"

/**
 * 程序动画工作方式:
 * 1. 在移动上， 调用时， 设置起点和终点，移动过程使用ProgramMoveCurve来计算移动百分比， 在ProgramMoveDuration时间内完成移动
 * 2. 在动画上， 通过KeyFrames来设置每个关键帧的时间和变换， 每个关键帧的变换是否相对于起始变换可配
 * 3. 重点问题: 当同时存在ProgramMoveCurve和KeyFrames时， 对于同时存在Location时， 将会把KeyFrames的数值附加到当前位置的结果上；
 *    因此一般而言，对于需要程序移动的动画，尾帧应该重置为0
 * 4. ProgramMoveCurve只会根据起点和终点来计算移动百分比, KeyFrames是附加在对应时间点位置上的。
 */
UCLASS(BlueprintType)
class LOMOLIB_API UProgramAnimationDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UProgramAnimationDataAsset();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta=(ShowOnlyInnerProperties))
	FProgramAnimationData AnimationData;
};