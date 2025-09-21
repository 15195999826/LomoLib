// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Animation/ProgramAnimationTypes.h"
#include "AnimationSmoothDataManager.generated.h"

/**
 * WorldSubsystem管理器：专门管理Smooth模式动画的预计算数据
 * 
 * 功能：
 * - 按需预计算动画切线数据
 * - 全局缓存，多个Component共享
 * - 自动验证数据有效性
 * - 编辑器友好的缓存管理
 */
UCLASS()
class LOMOLIB_API UAnimationSmoothDataManager : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	/**
	 * 获取或创建动画的预计算数据
	 * @param AnimationName 动画名称（用作缓存Key）
	 * @param AnimationData 动画数据（用于计算和验证）
	 * @return 预计算的切线数据，如果失败返回nullptr
	 */
	const FAnimationSmoothData* GetOrCreateSmoothData(const FName& AnimationName, const FProgramAnimationData& AnimationData);
	
	/**
	 * 清理所有缓存数据
	 */
	void ClearCache();
	
	/**
	 * 清理指定动画的缓存数据
	 */
	UFUNCTION(BlueprintCallable, Category = "Animation Smooth Data")
	void ClearAnimationCache(const FName& AnimationName);
	
	/**
	 * 获取缓存统计信息
	 */
	UFUNCTION(BlueprintCallable, Category = "Animation Smooth Data")
	int32 GetCachedAnimationCount() const { return SmoothDataCache.Num(); }
	
	/**
	 * 预计算指定动画的数据（可用于预热缓存）
	 */
	UFUNCTION(BlueprintCallable, Category = "Animation Smooth Data")
	bool PrecomputeAnimationData(const FName& AnimationName, const FProgramAnimationData& AnimationData);
	
private:
	// 动画名称 -> 预计算数据的缓存映射
	UPROPERTY()
	TMap<FName, FAnimationSmoothData> SmoothDataCache;
	
	/**
	 * 创建预计算数据
	 * @param AnimationData 源动画数据
	 * @return 新创建的预计算数据
	 */
	FAnimationSmoothData CreateSmoothData(const FProgramAnimationData& AnimationData);
	
	/**
	 * 计算Location向量的切线（复制自ProgramAniComponent的逻辑）
	 */
	FVector CalculateLocationTangent(const TArray<FProgramAnimationKeyFrame>& KeyFrames, int32 KeyIndex);
	
	/**
	 * 计算Scale向量的切线（复制自ProgramAniComponent的逻辑）
	 */
	FVector CalculateScaleTangent(const TArray<FProgramAnimationKeyFrame>& KeyFrames, int32 KeyIndex);
};