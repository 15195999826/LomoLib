// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Animation/ProgramAnimationTypes.h"
#include "AnimationSmoothDataManager.generated.h"


/**
 * 调试数据采样结构体
 */
USTRUCT()
struct LOMOLIB_API FAnimationDebugSamples
{
	GENERATED_BODY()
		
	// 位置采样数据
	UPROPERTY()
	TArray<FVector> LocationSamples;
		
	// 旋转采样数据
	UPROPERTY()
	TArray<FQuat> RotationSamples;
		
	// 缩放采样数据
	UPROPERTY()
	TArray<FVector> ScaleSamples;
		
	// 时间戳
	UPROPERTY()
	TArray<float> TimeStamps;
		
	// 调试显示控制参数
	bool bShowLocation = true;
	bool bShowRotation = true;
	bool bShowScale = true;
	int32 SampleCount = 100;
		
	FAnimationDebugSamples()
	{
		bShowLocation = true;
		bShowRotation = true;
		bShowScale = true;
		SampleCount = 100;
	}
		
	// 清空所有采样数据
	void Clear()
	{
		LocationSamples.Empty();
		RotationSamples.Empty();
		ScaleSamples.Empty();
		TimeStamps.Empty();
	}
		
	// 检查数据有效性
	bool IsValid() const
	{
		return LocationSamples.Num() > 0 && 
			   LocationSamples.Num() == RotationSamples.Num() && 
			   LocationSamples.Num() == ScaleSamples.Num() &&
			   LocationSamples.Num() == TimeStamps.Num();
	}
};


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

	FVector VectorHermiteInterpolate(const FVector& P0, const FVector& P1, const FVector& T0, const FVector& T1, 
	float Alpha) const;
	
	FTransform GetSmoothKeyFrameTransform(float ElapsedTime, const FProgramAnimationData& AnimationData, const FAnimationSmoothData* SmoothData);
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
	
#pragma region 调试可视化相关功能
private:
	
	// 调试数据缓存
	UPROPERTY()
	TMap<FName, FAnimationDebugSamples> DebugSamplesCache;
	
public:
	/**
	 * 可视化调试指定动画的Transform变化（从缓存获取数据）
	 * @param AnimationName 动画名称
	 * @param BaseLocation 调试显示的基准位置 
	 * @param SampleCount 采样点数量 (默认100)
	 * @param Duration 显示持续时间 (默认10秒)
	 */
	UFUNCTION(BlueprintCallable, Category = "Animation Debug")
	void DebugVisualizeAnimation(const FName& AnimationName, 
								const FVector& BaseLocation, 
								int32 SampleCount = 100,
								float Duration = 10.0f);
								
	/**
	 * 可视化调试动画的Transform变化（直接提供动画数据）
	 * @param AnimationData 动画数据
	 * @param BaseLocation 调试显示的基准位置 
	 * @param SampleCount 采样点数量 (默认100)
	 * @param Duration 显示持续时间 (默认10秒)
	 */
	UFUNCTION(BlueprintCallable, Category = "Animation Debug")
	void DebugVisualizeAnimationWithData(const FProgramAnimationData& AnimationData,
										const FVector& BaseLocation, 
										int32 SampleCount = 100,
										float Duration = 10.0f);
	
	/**
	 * 生成动画采样数据
	 * @param AnimationData 源动画数据
	 * @param SampleCount 采样点数量
	 * @return 生成的采样数据
	 */
	FAnimationDebugSamples GenerateAnimationSamples(const FProgramAnimationData& AnimationData, 
												   int32 SampleCount = 100);

private:
	// 可视化渲染函数
	void DrawLocationDebug(const TArray<FVector>& LocationSamples, const FVector& BasePos, float Duration);
	void DrawRotationDebug(const TArray<FQuat>& RotationSamples, const FVector& BasePos, float Duration);  
	void DrawScaleDebug(const TArray<FVector>& ScaleSamples, const FVector& BasePos, float Duration);
#pragma endregion 
};