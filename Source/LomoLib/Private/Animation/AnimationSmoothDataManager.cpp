// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/AnimationSmoothDataManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Settings/ProgramAnimationSettings.h"

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

FVector UAnimationSmoothDataManager::VectorHermiteInterpolate(const FVector& P0, const FVector& P1, 
	const FVector& T0, const FVector& T1, float Alpha) const
{
	float Alpha2 = Alpha * Alpha;
	float Alpha3 = Alpha2 * Alpha;
	
	// Hermite基函数
	float h1 = 2*Alpha3 - 3*Alpha2 + 1;   // P0权重
	float h2 = -2*Alpha3 + 3*Alpha2;      // P1权重  
	float h3 = Alpha3 - 2*Alpha2 + Alpha; // T0权重
	float h4 = Alpha3 - Alpha2;           // T1权重
	
	return h1*P0 + h2*P1 + h3*T0 + h4*T1;
}

FTransform UAnimationSmoothDataManager::GetSmoothKeyFrameTransform(float ElapsedTime, const FProgramAnimationData& AnimationData, const FAnimationSmoothData* SmoothData)
{
	if (AnimationData.KeyFrames.Num() == 0)
	{
		return FTransform::Identity;
	}

	const TArray<FProgramAnimationKeyFrame>& KeyFrames = AnimationData.KeyFrames;
	
	// 如果只有一个关键帧
	if (KeyFrames.Num() == 1)
	{
		if (ElapsedTime < KeyFrames[0].Time)
		{
			return FTransform::Identity; // 在唯一关键帧之前，返回初始状态
		}
		
		return KeyFrames[0].Transform;
	}

	// 查找当前时间在哪两个关键帧之间
	for (int32 i = 0; i < KeyFrames.Num() - 1; i++)
	{
		if (ElapsedTime >= KeyFrames[i].Time && ElapsedTime <= KeyFrames[i + 1].Time)
		{
			// 在两个关键帧之间进行插值
			float Alpha = (ElapsedTime - KeyFrames[i].Time) / (KeyFrames[i + 1].Time - KeyFrames[i].Time);
			
			// Smooth模式使用向量Hermite插值 - 真正的3D轨迹平滑
			if (AnimationData.CurveType == EProgramAnimationCurveType::Smooth)
			{
				const FTransform& T0 = KeyFrames[i].Transform;
				const FTransform& T1 = KeyFrames[i + 1].Transform;
				
				// 使用预计算的切线进行Location向量Hermite插值
				const FVector& LocationTangent0 = SmoothData->LocationTangents[i];
				const FVector& LocationTangent1 = SmoothData->LocationTangents[i + 1];
				FVector SmoothLocation = VectorHermiteInterpolate(
					T0.GetLocation(), T1.GetLocation(), 
					LocationTangent0, LocationTangent1, Alpha
				);
				
				// 使用预计算的切线进行Scale向量Hermite插值
				const FVector& ScaleTangent0 = SmoothData->ScaleTangents[i];
				const FVector& ScaleTangent1 = SmoothData->ScaleTangents[i + 1];
				FVector SmoothScale = VectorHermiteInterpolate(
					T0.GetScale3D(), T1.GetScale3D(),
					ScaleTangent0, ScaleTangent1, Alpha
				);
				
				// Rotation继续使用Slerp插值（四元数的最佳插值方法）
				FQuat SmoothRotation = FQuat::Slerp(T0.GetRotation(), T1.GetRotation(), Alpha);
				
				// 组装最终Transform
				FTransform Result;
				Result.SetLocation(SmoothLocation);
				Result.SetRotation(SmoothRotation);
				Result.SetScale3D(SmoothScale);
				return Result;
			}
		}
	}

	// 超出范围的情况
	if (ElapsedTime <= KeyFrames[0].Time)
	{
		return KeyFrames[0].Transform;
	}

	return KeyFrames.Last().Transform;
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

// ========== 调试功能实现 ==========

FAnimationDebugSamples UAnimationSmoothDataManager::GenerateAnimationSamples(
	const FProgramAnimationData& AnimationData, int32 SampleCount)
{
	FAnimationDebugSamples DebugSamples;
	DebugSamples.SampleCount = SampleCount;
	
	// 检查动画数据有效性
	if (AnimationData.KeyFrames.Num() < 1 || AnimationData.Duration <= 0.0f || SampleCount < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationSmoothDataManager: Invalid animation data for sampling"));
		return DebugSamples;
	}

	check(AnimationData.CurveType == EProgramAnimationCurveType::Smooth);
	
	// 预分配数组空间
	DebugSamples.LocationSamples.Reserve(SampleCount);
	DebugSamples.RotationSamples.Reserve(SampleCount);
	DebugSamples.ScaleSamples.Reserve(SampleCount);
	DebugSamples.TimeStamps.Reserve(SampleCount);
	
	// 获取或创建Smooth模式预计算数据（如果使用Smooth插值）
	const FAnimationSmoothData* SmoothData = GetOrCreateSmoothData(AnimationData.AnimationName, AnimationData);
	// 生成采样点
	for (int32 i = 0; i < SampleCount; i++)
	{
		// 计算采样时间点
		float Alpha = (SampleCount == 1) ? 0.0f : (float)i / (float)(SampleCount - 1);
		float SampleTime = Alpha * AnimationData.Duration;
		
		// 获取该时间点的关键帧变换
		FTransform KeyFrameTransform = GetSmoothKeyFrameTransform(SampleTime, AnimationData, SmoothData);
		
		// 存储采样数据
		DebugSamples.LocationSamples.Add(KeyFrameTransform.GetLocation());
		DebugSamples.RotationSamples.Add(KeyFrameTransform.GetRotation());
		DebugSamples.ScaleSamples.Add(KeyFrameTransform.GetScale3D());
		DebugSamples.TimeStamps.Add(SampleTime);
	}
	
	UE_LOG(LogTemp, Log, TEXT("AnimationSmoothDataManager: Generated %d debug samples for animation '%s'"), 
		SampleCount, *AnimationData.AnimationName.ToString());
	
	return DebugSamples;
}

// ========== 可视化渲染函数 ==========

void UAnimationSmoothDataManager::DrawLocationDebug(const TArray<FVector>& LocationSamples, 
	const FVector& BasePos, float Duration)
{
	if (LocationSamples.Num() < 2)
	{
		return;
	}
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	// 绘制位置轨迹连线
	for (int32 i = 0; i < LocationSamples.Num() - 1; i++)
	{
		FVector Point1 = BasePos + LocationSamples[i];
		FVector Point2 = BasePos + LocationSamples[i + 1];
		
		// 黄色连线，线宽3.0
		DrawDebugLine(World, Point1, Point2, FColor::Yellow, false, Duration, 0, 3.0f);
	}
	
	// 绘制关键采样点
	for (int32 i = 0; i < LocationSamples.Num(); i += FMath::Max(1, LocationSamples.Num() / 20)) // 最多显示20个点
	{
		FVector Point = BasePos + LocationSamples[i];
		DrawDebugSphere(World, Point, 2.0f, 8, FColor::Orange, false, Duration);
	}
}

void UAnimationSmoothDataManager::DrawRotationDebug(const TArray<FQuat>& RotationSamples, 
	const FVector& BasePos, float Duration)
{
	if (RotationSamples.Num() < 1)
	{
		return;
	}
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	const float ArrowLength = 60.0f;
	const float ArrowSize = 15.0f;
	const int32 StepSize = FMath::Max(1, RotationSamples.Num() / 30); // 最多显示30个方向
	
	for (int32 i = 0; i < RotationSamples.Num(); i += StepSize)
	{
		// 计算时间轴上的位置偏移 
		float TimeOffset = (float)i / (float)FMath::Max(1, RotationSamples.Num() - 1);
		FVector TimeAxisPos = BasePos + FVector(TimeOffset * 200.0f, 0, 0); // 200单位的时间轴
		
		// 获取旋转的三个轴向量
		FVector Forward = RotationSamples[i].GetForwardVector() * ArrowLength;
		FVector Right = RotationSamples[i].GetRightVector() * ArrowLength * 0.7f; 
		FVector Up = RotationSamples[i].GetUpVector() * ArrowLength * 0.5f;
		
		// 绘制三个方向的箭头
		DrawDebugDirectionalArrow(World, TimeAxisPos, TimeAxisPos + Forward, 
			ArrowSize, FColor::Red, false, Duration, 0, 2.0f);    // 前方向 - 红色
		DrawDebugDirectionalArrow(World, TimeAxisPos, TimeAxisPos + Right, 
			ArrowSize, FColor::Green, false, Duration, 0, 1.5f);  // 右方向 - 绿色  
		DrawDebugDirectionalArrow(World, TimeAxisPos, TimeAxisPos + Up, 
			ArrowSize, FColor::Blue, false, Duration, 0, 1.0f);   // 上方向 - 蓝色
			
		// 绘制时间轴基准点
		DrawDebugSphere(World, TimeAxisPos, 3.0f, 6, FColor::White, false, Duration);
	}
	
	// 绘制时间轴基准线
	if (RotationSamples.Num() > 1)
	{
		DrawDebugLine(World, BasePos, BasePos + FVector(200.0f, 0, 0), 
			FColor::Silver, false, Duration, 0, 1.0f);
	}
}

void UAnimationSmoothDataManager::DrawScaleDebug(const TArray<FVector>& ScaleSamples, 
	const FVector& BasePos, float Duration)
{
	if (ScaleSamples.Num() < 1)
	{
		return;
	}
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	const float MaxRayLength = 80.0f;
	const int32 StepSize = FMath::Max(1, ScaleSamples.Num() / 25); // 最多显示25个缩放状态
	
	for (int32 i = 0; i < ScaleSamples.Num(); i += StepSize)
	{
		// 时间轴位置
		float TimeOffset = (float)i / (float)FMath::Max(1, ScaleSamples.Num() - 1);
		FVector TimeAxisPos = BasePos + FVector(0, TimeOffset * 200.0f, 0); // Y轴时间轴
		
		const FVector& Scale = ScaleSamples[i];
		
		// 绘制三轴缩放射线，长度代表缩放值
		FVector XRay = FVector::ForwardVector * Scale.X * MaxRayLength;
		FVector YRay = FVector::RightVector * Scale.Y * MaxRayLength;
		FVector ZRay = FVector::UpVector * Scale.Z * MaxRayLength;
		
		// X轴缩放 - 红色，线宽根据缩放值变化
		float XThickness = FMath::Clamp(Scale.X * 2.0f, 0.5f, 5.0f);
		DrawDebugLine(World, TimeAxisPos, TimeAxisPos + XRay, 
			FColor::Red, false, Duration, 0, XThickness);
		
		// Y轴缩放 - 绿色
		float YThickness = FMath::Clamp(Scale.Y * 2.0f, 0.5f, 5.0f);
		DrawDebugLine(World, TimeAxisPos, TimeAxisPos + YRay, 
			FColor::Green, false, Duration, 0, YThickness);
		
		// Z轴缩放 - 蓝色
		float ZThickness = FMath::Clamp(Scale.Z * 2.0f, 0.5f, 5.0f);
		DrawDebugLine(World, TimeAxisPos, TimeAxisPos + ZRay, 
			FColor::Blue, false, Duration, 0, ZThickness);
		
		// 在射线端点显示数值（可选）
		if (i % (StepSize * 3) == 0) // 减少文字显示频率
		{
			FString ScaleText = FString::Printf(TEXT("%.2f,%.2f,%.2f"), Scale.X, Scale.Y, Scale.Z);
			DrawDebugString(World, TimeAxisPos + FVector(0, 0, 40), ScaleText, 
				nullptr, FColor::Cyan, Duration, false, 0.8f);
		}
		
		// 时间轴基准点
		DrawDebugSphere(World, TimeAxisPos, 2.0f, 6, FColor::Cyan, false, Duration);
	}
	
	// 绘制时间轴基准线 (Y轴)
	if (ScaleSamples.Num() > 1)
	{
		DrawDebugLine(World, BasePos, BasePos + FVector(0, 200.0f, 0), 
			FColor::Silver, false, Duration, 0, 1.0f);
	}
}

// ========== 主调试函数 ==========

void UAnimationSmoothDataManager::DebugVisualizeAnimation(const FName& AnimationName, 
	const FVector& BaseLocation, int32 SampleCount, float Duration)
{
	const auto& AnimationData = UProgramAnimationSettings::GetAnimationData(AnimationName);

	DebugVisualizeAnimationWithData(AnimationData, BaseLocation, SampleCount, Duration);
}

void UAnimationSmoothDataManager::DebugVisualizeAnimationWithData(const FProgramAnimationData& AnimationData,
	const FVector& BaseLocation, int32 SampleCount, float Duration)
{
	if (AnimationData.CurveType != EProgramAnimationCurveType::Smooth)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, 
			FString::Printf(TEXT("Animation '%s' 不是Smooth模式，无法进行平滑调试可视化"), 
			*AnimationData.AnimationName.ToString()));
		return;
	}
	
	if (AnimationData.KeyFrames.Num() < 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimationSmoothDataManager: Invalid animation data for debugging"));
		return;
	}
	
	// 生成调试采样数据
	FAnimationDebugSamples DebugSamples = GenerateAnimationSamples(AnimationData, SampleCount);
	
	if (!DebugSamples.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("AnimationSmoothDataManager: Failed to generate debug samples"));
		return;
	}
	
	// 缓存调试采样数据
	DebugSamplesCache.Add(AnimationData.AnimationName, DebugSamples);
	
	UE_LOG(LogTemp, Log, TEXT("AnimationSmoothDataManager: Visualizing animation '%s' with %d samples at location %s"), 
		*AnimationData.AnimationName.ToString(), DebugSamples.LocationSamples.Num(), *BaseLocation.ToString());
	
	// 根据控制参数绘制不同部分
	if (DebugSamples.bShowLocation)
	{
		DrawLocationDebug(DebugSamples.LocationSamples, BaseLocation, Duration);
	}
	
	if (DebugSamples.bShowRotation)
	{
		// 旋转调试在BaseLocation右侧偏移
		FVector RotationBasePos = BaseLocation + FVector(0, 250.0f, 0);
		DrawRotationDebug(DebugSamples.RotationSamples, RotationBasePos, Duration);
	}
	
	if (DebugSamples.bShowScale)
	{
		// 缩放调试在BaseLocation左侧偏移
		FVector ScaleBasePos = BaseLocation + FVector(0, -250.0f, 0);
		DrawScaleDebug(DebugSamples.ScaleSamples, ScaleBasePos, Duration);
	}
}
