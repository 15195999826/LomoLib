// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/AnimationSmoothDataManager.h"
#include "Engine/World.h"

void UAnimationSmoothDataManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Log, TEXT("AnimationSmoothDataManager: Initialized"));
}

void UAnimationSmoothDataManager::Deinitialize()
{
	// 清理所有缓存数据
	ClearCache();
	
	Super::Deinitialize();
}

const FAnimationSmoothData* UAnimationSmoothDataManager::GetOrCreateSmoothData(const FName& AnimationName, const FProgramAnimationData& AnimationData)
{
	// 检查动画数据是否有效
	if (AnimationData.KeyFrames.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationSmoothDataManager: Animation '%s' has insufficient keyframes (%d)"), 
			*AnimationName.ToString(), AnimationData.KeyFrames.Num());
		return nullptr;
	}
	
	// 检查缓存是否存在且有效
	if (FAnimationSmoothData* CachedData = SmoothDataCache.Find(AnimationName))
	{
		// 缓存命中且数据匹配
		return CachedData;
	}
	
	// 创建新的预计算数据
	FAnimationSmoothData NewSmoothData = CreateSmoothData(AnimationData);
	
	if (!NewSmoothData.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationSmoothDataManager: Failed to create smooth data for animation '%s'"), 
			*AnimationName.ToString());
		return nullptr;
	}
	
	// 缓存新数据
	SmoothDataCache.Add(AnimationName, NewSmoothData);
	
	UE_LOG(LogTemp, Log, TEXT("AnimationSmoothDataManager: Created smooth data for animation '%s' with %d keyframes"), 
		*AnimationName.ToString(), NewSmoothData.KeyFrameCount);
	
	return SmoothDataCache.Find(AnimationName);
}

void UAnimationSmoothDataManager::ClearCache()
{
	int32 ClearedCount = SmoothDataCache.Num();
	SmoothDataCache.Empty();
	
	UE_LOG(LogTemp, Log, TEXT("AnimationSmoothDataManager: Cleared %d cached animations"), ClearedCount);
}

void UAnimationSmoothDataManager::ClearAnimationCache(const FName& AnimationName)
{
	if (SmoothDataCache.Remove(AnimationName) > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("AnimationSmoothDataManager: Cleared cache for animation '%s'"), 
			*AnimationName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationSmoothDataManager: Animation '%s' not found in cache"), 
			*AnimationName.ToString());
	}
}

bool UAnimationSmoothDataManager::PrecomputeAnimationData(const FName& AnimationName, const FProgramAnimationData& AnimationData)
{
	const FAnimationSmoothData* Result = GetOrCreateSmoothData(AnimationName, AnimationData);
	return Result != nullptr;
}

FAnimationSmoothData UAnimationSmoothDataManager::CreateSmoothData(const FProgramAnimationData& AnimationData)
{
	FAnimationSmoothData SmoothData;
	
	const int32 KeyFrameCount = AnimationData.KeyFrames.Num();
	if (KeyFrameCount < 2)
	{
		return SmoothData; // 返回无效数据
	}
	
	// 初始化数据
	SmoothData.KeyFrameCount = KeyFrameCount;
	SmoothData.Duration = AnimationData.Duration;
	SmoothData.LocationTangents.SetNum(KeyFrameCount);
	SmoothData.ScaleTangents.SetNum(KeyFrameCount);
	SmoothData.KeyFrameTimes.SetNum(KeyFrameCount);
	
	// 复制关键帧时间戳
	for (int32 i = 0; i < KeyFrameCount; i++)
	{
		SmoothData.KeyFrameTimes[i] = AnimationData.KeyFrames[i].Time;
	}
	
	// 预计算所有关键帧的切线
	for (int32 i = 0; i < KeyFrameCount; i++)
	{
		SmoothData.LocationTangents[i] = CalculateLocationTangent(AnimationData.KeyFrames, i);
		SmoothData.ScaleTangents[i] = CalculateScaleTangent(AnimationData.KeyFrames, i);
	}
	
	return SmoothData;
}

FVector UAnimationSmoothDataManager::CalculateLocationTangent(const TArray<FProgramAnimationKeyFrame>& KeyFrames, int32 KeyIndex)
{
	if (KeyFrames.Num() < 2) return FVector::ZeroVector;
	
	if (KeyIndex == 0)
	{
		// 第一个关键帧：使用与下一个关键帧的方向
		return KeyFrames[1].Transform.GetLocation() - KeyFrames[0].Transform.GetLocation();
	}

	if (KeyIndex == KeyFrames.Num() - 1)
	{
		// 最后一个关键帧：使用与前一个关键帧的方向
		return KeyFrames[KeyIndex].Transform.GetLocation() - KeyFrames[KeyIndex-1].Transform.GetLocation();
	}

	// 中间关键帧：使用前后关键帧的平均方向，确保C1连续性
	FVector PrevToThis = KeyFrames[KeyIndex].Transform.GetLocation() - KeyFrames[KeyIndex-1].Transform.GetLocation();
	FVector ThisToNext = KeyFrames[KeyIndex+1].Transform.GetLocation() - KeyFrames[KeyIndex].Transform.GetLocation();
		
	// 根据时间间隔加权平均，保证速度连续性
	float PrevTimeDiff = KeyFrames[KeyIndex].Time - KeyFrames[KeyIndex-1].Time;
	float NextTimeDiff = KeyFrames[KeyIndex+1].Time - KeyFrames[KeyIndex].Time;
		
	if (PrevTimeDiff > 0 && NextTimeDiff > 0)
	{
		// 标准化为单位时间的速度，然后加权平均
		FVector PrevVelocity = PrevToThis / PrevTimeDiff;
		FVector NextVelocity = ThisToNext / NextTimeDiff;
		FVector AvgVelocity = (PrevVelocity + NextVelocity) * 0.5f;
			
		// 转换回当前时间段的切线长度
		return AvgVelocity * NextTimeDiff;
	}

	// 备用方案：简单平均
	return (ThisToNext + PrevToThis) * 0.5f;
}

FVector UAnimationSmoothDataManager::CalculateScaleTangent(const TArray<FProgramAnimationKeyFrame>& KeyFrames, int32 KeyIndex)
{
	if (KeyFrames.Num() < 2) return FVector::ZeroVector;
	
	if (KeyIndex == 0)
	{
		return KeyFrames[1].Transform.GetScale3D() - KeyFrames[0].Transform.GetScale3D();
	}

	if (KeyIndex == KeyFrames.Num() - 1)
	{
		return KeyFrames[KeyIndex].Transform.GetScale3D() - KeyFrames[KeyIndex-1].Transform.GetScale3D();
	}

	FVector PrevToThis = KeyFrames[KeyIndex].Transform.GetScale3D() - KeyFrames[KeyIndex-1].Transform.GetScale3D();
	FVector ThisToNext = KeyFrames[KeyIndex+1].Transform.GetScale3D() - KeyFrames[KeyIndex].Transform.GetScale3D();
		
	// 时间加权平均（与Location切线计算逻辑相同）
	float PrevTimeDiff = KeyFrames[KeyIndex].Time - KeyFrames[KeyIndex-1].Time;
	float NextTimeDiff = KeyFrames[KeyIndex+1].Time - KeyFrames[KeyIndex].Time;
		
	if (PrevTimeDiff > 0 && NextTimeDiff > 0)
	{
		FVector PrevVelocity = PrevToThis / PrevTimeDiff;
		FVector NextVelocity = ThisToNext / NextTimeDiff;
		FVector AvgVelocity = (PrevVelocity + NextVelocity) * 0.5f;
		return AvgVelocity * NextTimeDiff;
	}

	return (ThisToNext + PrevToThis) * 0.5f;
}
