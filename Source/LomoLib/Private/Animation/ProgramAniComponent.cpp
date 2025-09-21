// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/ProgramAniComponent.h"
#include "Animation/ProgramAnimationDataAsset.h"
#include "Animation/AnimationSmoothDataManager.h"
#include "Settings/ProgramAnimationSettings.h"
#include "WaitGroupManager.h"

const FProgramAnimationParams FProgramAnimationParams::Invalid{ };
const FProgramAnimationParams FProgramAnimationParams::ForceShow{true};

// Sets default values for this component's properties
UProgramAniComponent::UProgramAniComponent(): CachedStartLocation(), CachedEndLocation(),
                                                          HoveringWaitGroupID(0)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UProgramAniComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UProgramAniComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UProgramAniComponent::BindToSceneComponent(USceneComponent* InComponent)
{
	OwnerSceneComponent = InComponent;
}

int32 UProgramAniComponent::PlayProgramAnimationByName(const FName& InAniName, const FVector& StartLocation,
                                                       const FVector& EndLocation, bool bRelativeToParent, float StartProgress, const FProgramAnimationParams& Params)
{
	// 通过动画设置获取动画数据
	return PlayProgramAnimation(UProgramAnimationSettings::GetAnimationData(InAniName), StartLocation, EndLocation, bRelativeToParent, StartProgress, true, Params);
}

int32 UProgramAniComponent::ReverseLastAnimation(bool bAssignNewEndLocation, const FVector& NewEndLocation)
{
	if (CurrentAnimationData.bUseAnchorMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] ReverseLastAnimation: Anchor mode is not supported for reversing animations"),
			*GetOwner()->GetName());
		return INDEX_NONE;
	}
	
	// 反向播放上一个动画
	check(!bIsPlayingAnimation)

	FString DesiredAnimationName;
	auto CurrentAnimationDataNameStr = CurrentAnimationData.AnimationName.ToString();
	if (CurrentAnimationDataNameStr.EndsWith("_Reverse"))
	{
		DesiredAnimationName = CurrentAnimationDataNameStr.LeftChop(8); // 去掉"_Reverse"
	}
	else
	{
		DesiredAnimationName = CurrentAnimationDataNameStr + "_Reverse"; // 添加"_Reverse"
	}

	// 生成反向动画数据
	const auto& AnimationData = FProgramAnimationData::GenerateReverseAnimation(CurrentAnimationData);
	
	// CachedStartLocation 只跟程序移动相关
	auto TempStartLocation = CachedStartLocation;
	CachedStartLocation = CachedEndLocation;
	if (bAssignNewEndLocation)
	{
		CachedEndLocation = NewEndLocation;
	}
	else
	{
		CachedEndLocation = TempStartLocation;
	}
	
	CachedInitialTransform = CachedLastAnimationInitialTransform;

	USceneComponent* TargetComponent = GetTargetTransformComponent();
	FVector CurrentLocation;
	if (bLastRelativeToParent)
	{
		CurrentLocation = TargetComponent->GetRelativeLocation();
	}
	else
	{
		CurrentLocation = TargetComponent->GetComponentLocation();
	}
	
	// 重新播放反转动画
	// 直接从当前位置开始新的反转动画，进度从0开始
	return PlayProgramAnimation(AnimationData, CurrentLocation, TempStartLocation, bLastRelativeToParent, 0, false);
}

int32 UProgramAniComponent::InterruptAndReverseCurrentAnimation(bool bCancelWaitGroup)
{
	USceneComponent* TargetComponent = GetTargetTransformComponent();
	FVector CurrentLocation;
	if (bIsRelativeToParent)
	{
		CurrentLocation = TargetComponent->GetRelativeLocation();
	}
	else
	{
		CurrentLocation = TargetComponent->GetComponentLocation();
	}
	
	return InterruptAndReverseCurrentAnimation(CurrentLocation, CachedStartLocation, bCancelWaitGroup);
}

int32 UProgramAniComponent::InterruptAndReverseCurrentAnimation(const FVector& InCurrentLocation,
                                                                      const FVector& EndLocation,
                                                                      bool bCancelWaitGroup)
{
	check(bIsPlayingAnimation);
	if (bCancelWaitGroup)
	{
		auto WGManager = GetWorld()->GetSubsystem<UWaitGroupManager>();
		auto WG = WGManager->FindWaitGroup(HoveringWaitGroupID);
		if (WG)
		{
			WG->SetCancelled();
		}
	}
	
	// 保存当前动画重要数据
	FString DesiredAnimationName;
	auto CurrentAnimationDataNameStr = CurrentAnimationData.AnimationName.ToString();
	if (CurrentAnimationDataNameStr.EndsWith("_Reverse"))
	{
		DesiredAnimationName = CurrentAnimationDataNameStr.LeftChop(8); // 去掉"_Reverse"
	}
	else
	{
		DesiredAnimationName = CurrentAnimationDataNameStr + "_Reverse"; // 添加"_Reverse"
	}

	// 生成反向动画数据
	const auto& AnimationData = FProgramAnimationData::GenerateReverseAnimation(CurrentAnimationData);

	// 计算当前动画的实际进度
	auto CurrentProgress = (GetWorld()->GetTimeSeconds() - AnimationStartTime) / CurrentAnimationData.Duration;
	CurrentProgress = FMath::Clamp(CurrentProgress, 0.0f, 1.0f);
	
	// 对于反转动画，需要计算映射后的进度
	// 反转动画的进度应该是: 从当前位置开始，映射到反转动画的对应时间点
	auto DesiredProgress = 1.0f - CurrentProgress;

	auto TempCurrentLocation = InCurrentLocation;
	auto TempEndLocation = EndLocation;
	// 设置反转动画的初始Transform
	auto TempCachedInitialTransform = CachedInitialTransform;
	// 根据EndLocation和StartLocation计算新的起始位置, 这里传入的位置是逆向动画的播放位置， 但需要更新CachedStartLocation为， 这条反转动画的起始位置
	auto CurveStartLocation = SimulateStartTransform(AnimationData, EndLocation, InCurrentLocation, DesiredProgress);
	auto TempBIsRelativeToParent = bIsRelativeToParent;
	

	StopAnimation();

	CachedInitialTransform = TempCachedInitialTransform;
	// CachedStartLocation 只跟程序移动相关
	CachedStartLocation = CurveStartLocation;
	CachedEndLocation = TempEndLocation;

	// 重新播放反转动画
	// 直接从当前位置开始新的反转动画，进度从0开始
	return PlayProgramAnimation(AnimationData, TempCurrentLocation, TempEndLocation, TempBIsRelativeToParent, DesiredProgress, false);
}

int32 UProgramAniComponent::PlayProgramAnimation(const FProgramAnimationData& AnimationData, 
                                                       const FVector& StartLocation, 
                                                       const FVector& EndLocation,
                                                       bool bRelativeToParent,
                                                       float StartProgress,
                                                       bool bAutoInitialCache,
                                                       const FProgramAnimationParams& Params)
{
	if (bIsPlayingAnimation)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] PlayAnimation: Already playing animation: %s, cannot play new animation: %s"),
			*GetOwner()->GetName(),
			*CurrentAnimationData.AnimationName.ToString(),
			*AnimationData.AnimationName.ToString());
		return INDEX_NONE;
	}
	
	HoveringWaitGroupID = INDEX_NONE;
	
	// 设置锚点模式 - 从动画数据读取
	bIsAnchorMode = AnimationData.bUseAnchorMode;
	
	// 设置相对坐标模式
	bIsRelativeToParent = bRelativeToParent;
	
	// 记录初始Transform状态（在设置位置之前）
	if (bAutoInitialCache)
	{
		USceneComponent* TargetComponent = GetTargetTransformComponent();
		if (bIsRelativeToParent)
		{
			CachedInitialTransform = TargetComponent->GetRelativeTransform();
		}
		else
		{
			CachedInitialTransform = TargetComponent->GetComponentTransform();
		}
		CachedStartLocation = StartLocation;
		CachedEndLocation = EndLocation;
	}
	
	// 缓存动画数据
	CurrentAnimationData = AnimationData;

	// 动画数据安全性校验（使用浮点数容差比较）
	float MoveEndTime = AnimationData.ProgramMoveStartTime + AnimationData.ProgramMoveDuration;
	if (MoveEndTime > AnimationData.Duration + KINDA_SMALL_NUMBER)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] 动画%s配置错误, 强制修正程序移动时间, 总时长: %f, 程序移动开始时间: %f, 程序移动持续时间: %f, 总时长<= 程序移动开始时间 + 程序移动持续时间%s"),
			*GetOwner()->GetName(),
			*AnimationData.AnimationName.ToString(),
			AnimationData.Duration,
			AnimationData.ProgramMoveStartTime,
			AnimationData.ProgramMoveDuration,
			AnimationData.Duration <= AnimationData.ProgramMoveStartTime + AnimationData.ProgramMoveDuration ? TEXT("，已修正") : TEXT(""));
		if (AnimationData.ProgramMoveStartTime > AnimationData.Duration)
		{
			CurrentAnimationData.ProgramMoveStartTime = AnimationData.Duration;
			CurrentAnimationData.ProgramMoveDuration = 0.f;
		}
		else
		{
			CurrentAnimationData.ProgramMoveDuration = AnimationData.Duration - AnimationData.ProgramMoveStartTime;
		}
	}
	
	CurrentAnimationData.bIsValid = true;
	
	bIsPlayingAnimation = true;
	// 根据StartProgress调整开始时间
	float CurrentTime = GetWorld()->GetTimeSeconds();
	AnimationStartTime = (StartProgress > 0.0f) 
		? CurrentTime - (StartProgress * CurrentAnimationData.Duration)
		: CurrentTime;

	// 设置初始位置
	if (bIsAnchorMode)
	{
		// 锚点模式：先设置到StartLocation，然后缓存锚点，再计算正确的初始位置
		USceneComponent* TargetComponent = GetTargetTransformComponent();
		if (bIsRelativeToParent)
		{
			TargetComponent->SetRelativeLocation(StartLocation);
		}
		else
		{
			TargetComponent->SetWorldLocation(StartLocation);
		}
		
		// 缓存锚点位置（基于刚设置的StartLocation）
		CacheAnchorLocations();
		
		// 计算并设置基于锚点的初始位置
		FTransform InitialAnchorTransform = CalculateAnchorModeTransform(StartProgress * CurrentAnimationData.Duration);
		if (bIsRelativeToParent)
		{
			TargetComponent->SetRelativeTransform(InitialAnchorTransform);
		}
		else
		{
			TargetComponent->SetWorldTransform(InitialAnchorTransform);
		}
	}
	else
	{
		// 传统模式：使用程序移动的起始位置
		USceneComponent* TargetComponent = GetTargetTransformComponent();
		if (bIsRelativeToParent)
		{
			TargetComponent->SetRelativeLocation(StartLocation);
		}
		else
		{
			TargetComponent->SetWorldLocation(StartLocation);
		}
	}

	// 处理Duration为0的情况 - 直接跳转到结束状态
	if (CurrentAnimationData.Duration <= 0.0f)
	{
		// 计算最终Transform并直接应用
		FTransform FinalTransform;
		if (bIsAnchorMode)
		{
			// 锚点模式需要先缓存锚点（如果还没缓存的话）
			if (CachedAnchorSegments.Num() == 0)
			{
				CacheAnchorLocations();
			}
			FinalTransform = CalculateAnchorModeTransform(0.0f);
		}
		else
		{
			FinalTransform = CalculateFinalTransform();
		}
		
		USceneComponent* TargetComponent = GetTargetTransformComponent();
		if (bIsRelativeToParent)
		{
			TargetComponent->SetRelativeTransform(FinalTransform);
		}
		else
		{
			TargetComponent->SetWorldTransform(FinalTransform);
		}
		
		// 立即结束动画
		bIsPlayingAnimation = false;
		CurrentAnimationData.bIsValid = false;
		bIsAnchorMode = false;
		CachedAnchorSegments.Empty();
		return INDEX_NONE;
	}

	auto WGTuple = GetWorld()->GetSubsystem<UWaitGroupManager>()->CreateWaitGroup("ProgramAnimation");
	WGTuple.Value->Add();
	
	HoveringWaitGroupID = WGTuple.Key;
	
	// 启动Timer更新 (120fps)
	GetWorld()->GetTimerManager().SetTimer(
		AnimationTimerHandle,
		this,
		&UProgramAniComponent::UpdateAnimation,
		0.0083333333333333f,
		FTimerManagerTimerParameters { .bLoop = true, .bMaxOncePerFrame = true }
	);

	if (Params.bForceVisibleAtStart)
	{
		FTimerHandle UnusedHandle;
		GetWorld()->GetTimerManager().SetTimer(
			UnusedHandle,
			[this]()
			{
				if (GetOwner()->IsHidden())
				{
					GetOwner()->SetActorHiddenInGame(false);
				}
			},
			0.009f,
			false
		);
	}

	return HoveringWaitGroupID;
}

void UProgramAniComponent::StopAnimation()
{
	if (!bIsPlayingAnimation)
	{
		return;
	}

	bLastRelativeToParent = bIsRelativeToParent;
	CachedLastAnimationInitialTransform = CachedInitialTransform;

	// 清理状态
	bIsPlayingAnimation = false;
	CurrentAnimationData.bIsValid = false;
	CachedInitialTransform = FTransform::Identity;
	bIsRelativeToParent = false;
	
	// 清理锚点模式状态
	bIsAnchorMode = false;
	CachedAnchorSegments.Empty();
	
	// 清理Timer
	GetWorld()->GetTimerManager().ClearTimer(AnimationTimerHandle);

	OnAnimationEnd.Broadcast();

	// 完成任务
	if (HoveringWaitGroupID == INDEX_NONE)
	{
		return;
	}

	auto WG = GetWorld()->GetSubsystem<UWaitGroupManager>()->FindWaitGroup(HoveringWaitGroupID);
	WG->Done(NAME_None, false);
}

bool UProgramAniComponent::IsPlayingReverseAnimation() const
{
	if (!bIsPlayingAnimation)
	{
		return false;
	}

	auto CurrentAnimationDataNameStr = CurrentAnimationData.AnimationName.ToString();
	return CurrentAnimationDataNameStr.EndsWith("_Reverse");
}

float UProgramAniComponent::GetProgramMoveAlpha(float ElapsedTime) const
{
	if (CurrentAnimationData.ProgramMoveDuration <= 0.0f)
	{
		return 1.0f;
	}
	
	// 检查是否在程序移动时间范围内
	if (ElapsedTime < CurrentAnimationData.ProgramMoveStartTime)
	{
		return 0.0f; // 程序移动还未开始
	}
	
	float ProgramMoveElapsedTime = ElapsedTime - CurrentAnimationData.ProgramMoveStartTime;
	return FMath::Clamp(ProgramMoveElapsedTime / CurrentAnimationData.ProgramMoveDuration, 0.0f, 1.0f);
}

FTransform UProgramAniComponent::GetKeyFrameTransform(float ElapsedTime, const FProgramAnimationData& AnimationData) const
{
	if (AnimationData.KeyFrames.Num() == 0)
	{
		return FTransform::Identity;
	}

	const TArray<FProgramAnimationKeyFrame>& KeyFrames = AnimationData.KeyFrames;
	
	// 如果只有一个关键帧
	if (KeyFrames.Num() == 1)
	{
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
				// 获取预计算数据管理器
				auto* SmoothManager = GetWorld()->GetSubsystem<UAnimationSmoothDataManager>();
				if (!SmoothManager)
				{
					UE_LOG(LogTemp, Error, TEXT("ProgramAniComponent: AnimationSmoothDataManager not available, falling back to linear interpolation"));
					// 降级为线性插值
					FTransform Result;
					Result.Blend(KeyFrames[i].Transform, KeyFrames[i + 1].Transform, Alpha);
					return Result;
				}
				
				// 获取或创建预计算数据
				const FAnimationSmoothData* SmoothData = SmoothManager->GetOrCreateSmoothData(AnimationData.AnimationName, AnimationData);
				if (!SmoothData || !SmoothData->IsValid())
				{
					UE_LOG(LogTemp, Warning, TEXT("ProgramAniComponent: Failed to get smooth data for animation '%s', falling back to linear interpolation"), 
						*AnimationData.AnimationName.ToString());
					// 降级为线性插值
					FTransform Result;
					Result.Blend(KeyFrames[i].Transform, KeyFrames[i + 1].Transform, Alpha);
					return Result;
				}
				
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
			else
			{
				// 其他模式使用原有逻辑
				Alpha = ApplyCurveType(Alpha, AnimationData.CurveType);
				
				FTransform Result;
				Result.Blend(KeyFrames[i].Transform, KeyFrames[i + 1].Transform, Alpha);
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

float UProgramAniComponent::ApplyCurveType(float Alpha, EProgramAnimationCurveType CurveType) const
{
	switch (CurveType)
	{
	case EProgramAnimationCurveType::Linear:
		return Alpha;
	case EProgramAnimationCurveType::EaseIn:
		return Alpha * Alpha;
	case EProgramAnimationCurveType::EaseOut:
		return 1.0f - FMath::Pow(1.0f - Alpha, 2.0f);
	case EProgramAnimationCurveType::EaseInOut:
		return Alpha < 0.5f ? 2.0f * Alpha * Alpha : 1.0f - FMath::Pow(-2.0f * Alpha + 2.0f, 2.0f) / 2.0f;
	case EProgramAnimationCurveType::Bezier:
		// 简单的3次贝塞尔曲线近似
		return Alpha * Alpha * (3.0f - 2.0f * Alpha);
	case EProgramAnimationCurveType::Smooth:
		// Smooth模式使用特殊的多关键帧插值，这里暂时返回Alpha
		// 实际插值在关键帧处理中进行
		return Alpha;
	default:
		return Alpha;
	}
}

// Hermite插值核心函数 - 对标UE5 RCIM_Cubic
float UProgramAniComponent::HermiteInterpolate(float P0, float P1, float T0, float T1, float Alpha) const
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

// 向量Hermite插值 - 真正的3D轨迹平滑
FVector UProgramAniComponent::VectorHermiteInterpolate(const FVector& P0, const FVector& P1, const FVector& T0, const FVector& T1, float Alpha) const
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

// 计算Location向量的切线
FVector UProgramAniComponent::CalculateLocationTangent(const TArray<FProgramAnimationKeyFrame>& KeyFrames, int32 KeyIndex) const
{
	if (KeyFrames.Num() < 2) return FVector::ZeroVector;
	
	if (KeyIndex == 0)
	{
		// 第一个关键帧：使用与下一个关键帧的方向
		return KeyFrames[1].Transform.GetLocation() - KeyFrames[0].Transform.GetLocation();
	}
	else if (KeyIndex == KeyFrames.Num() - 1)
	{
		// 最后一个关键帧：使用与前一个关键帧的方向
		return KeyFrames[KeyIndex].Transform.GetLocation() - KeyFrames[KeyIndex-1].Transform.GetLocation();
	}
	else
	{
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
		else
		{
			// 备用方案：简单平均
			return (ThisToNext + PrevToThis) * 0.5f;
		}
	}
}

// 计算Scale向量的切线
FVector UProgramAniComponent::CalculateScaleTangent(const TArray<FProgramAnimationKeyFrame>& KeyFrames, int32 KeyIndex) const
{
	if (KeyFrames.Num() < 2) return FVector::ZeroVector;
	
	if (KeyIndex == 0)
	{
		return KeyFrames[1].Transform.GetScale3D() - KeyFrames[0].Transform.GetScale3D();
	}
	else if (KeyIndex == KeyFrames.Num() - 1)
	{
		return KeyFrames[KeyIndex].Transform.GetScale3D() - KeyFrames[KeyIndex-1].Transform.GetScale3D();
	}
	else
	{
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
		else
		{
			return (ThisToNext + PrevToThis) * 0.5f;
		}
	}
}

// 计算平滑的时间Alpha - 基于关键帧时间的Hermite插值
float UProgramAniComponent::CalculateSmoothTimeAlpha(const TArray<FProgramAnimationKeyFrame>& KeyFrames, int32 CurrentKeyIndex, float RawAlpha) const
{
	if (KeyFrames.Num() < 2) return RawAlpha;
	
	// 计算时间轴上的切线（基于相邻关键帧的时间间隔）
	float InTangent = 0.0f;
	float OutTangent = 0.0f;
	
	// 计算当前关键帧段的进入切线
	if (CurrentKeyIndex > 0)
	{
		float PrevTimeDiff = KeyFrames[CurrentKeyIndex].Time - KeyFrames[CurrentKeyIndex - 1].Time;
		float CurrentTimeDiff = KeyFrames[CurrentKeyIndex + 1].Time - KeyFrames[CurrentKeyIndex].Time;
		InTangent = (PrevTimeDiff + CurrentTimeDiff) * 0.5f; // 平均时间间隔作为切线权重
	}
	else
	{
		InTangent = KeyFrames[CurrentKeyIndex + 1].Time - KeyFrames[CurrentKeyIndex].Time;
	}
	
	// 计算下一个关键帧段的离开切线  
	if (CurrentKeyIndex + 1 < KeyFrames.Num() - 1)
	{
		float CurrentTimeDiff = KeyFrames[CurrentKeyIndex + 1].Time - KeyFrames[CurrentKeyIndex].Time;
		float NextTimeDiff = KeyFrames[CurrentKeyIndex + 2].Time - KeyFrames[CurrentKeyIndex + 1].Time;
		OutTangent = (CurrentTimeDiff + NextTimeDiff) * 0.5f;
	}
	else
	{
		OutTangent = KeyFrames[CurrentKeyIndex + 1].Time - KeyFrames[CurrentKeyIndex].Time;
	}
	
	// 使用Hermite插值计算平滑的时间进度
	// P0=0, P1=1 (归一化的段内进度)，T0和T1为切线权重
	float NormalizedTangent0 = InTangent * 0.1f;  // 缩放切线影响
	float NormalizedTangent1 = OutTangent * 0.1f;
	
	return HermiteInterpolate(0.0f, 1.0f, NormalizedTangent0, NormalizedTangent1, RawAlpha);
}

// 计算关键帧的切线（基于相邻关键帧）
float UProgramAniComponent::CalculateKeyFrameTangent(const TArray<FProgramAnimationKeyFrame>& KeyFrames, int32 KeyIndex, bool bIsInTangent, int32 ComponentIndex) const
{
	if (KeyFrames.Num() < 2) return 0.0f;
	
	// 获取关键帧的Transform分量值
	auto GetComponentValue = [](const FTransform& Transform, int32 CompIndex) -> float
	{
		switch (CompIndex)
		{
			case 0: return Transform.GetLocation().X;
			case 1: return Transform.GetLocation().Y;
			case 2: return Transform.GetLocation().Z;
			case 3: return Transform.GetRotation().X;
			case 4: return Transform.GetRotation().Y;
			case 5: return Transform.GetRotation().Z;
			case 6: return Transform.GetRotation().W;
			case 7: return Transform.GetScale3D().X;
			case 8: return Transform.GetScale3D().Y;
			case 9: return Transform.GetScale3D().Z;
			default: return 0.0f;
		}
	};
	
	// 边界处理
	if (KeyIndex == 0)
	{
		// 第一个关键帧：使用与下一个关键帧的差值
		if (KeyFrames.Num() > 1)
		{
			float TimeDiff = KeyFrames[1].Time - KeyFrames[0].Time;
			float ValueDiff = GetComponentValue(KeyFrames[1].Transform, ComponentIndex) - GetComponentValue(KeyFrames[0].Transform, ComponentIndex);
			return TimeDiff > 0 ? ValueDiff / TimeDiff : 0.0f;
		}
		return 0.0f;
	}
	else if (KeyIndex == KeyFrames.Num() - 1)
	{
		// 最后一个关键帧：使用与前一个关键帧的差值
		float TimeDiff = KeyFrames[KeyIndex].Time - KeyFrames[KeyIndex-1].Time;
		float ValueDiff = GetComponentValue(KeyFrames[KeyIndex].Transform, ComponentIndex) - GetComponentValue(KeyFrames[KeyIndex-1].Transform, ComponentIndex);
		return TimeDiff > 0 ? ValueDiff / TimeDiff : 0.0f;
	}
	else
	{
		// 中间关键帧：使用前后关键帧的平均斜率
		float PrevTimeDiff = KeyFrames[KeyIndex].Time - KeyFrames[KeyIndex-1].Time;
		float NextTimeDiff = KeyFrames[KeyIndex+1].Time - KeyFrames[KeyIndex].Time;
		float PrevValueDiff = GetComponentValue(KeyFrames[KeyIndex].Transform, ComponentIndex) - GetComponentValue(KeyFrames[KeyIndex-1].Transform, ComponentIndex);
		float NextValueDiff = GetComponentValue(KeyFrames[KeyIndex+1].Transform, ComponentIndex) - GetComponentValue(KeyFrames[KeyIndex].Transform, ComponentIndex);
		
		float PrevSlope = PrevTimeDiff > 0 ? PrevValueDiff / PrevTimeDiff : 0.0f;
		float NextSlope = NextTimeDiff > 0 ? NextValueDiff / NextTimeDiff : 0.0f;
		
		// 返回平均斜率
		return (PrevSlope + NextSlope) * 0.5f;
	}
}

FTransform UProgramAniComponent::CalculateFinalTransform() const
{
	// 基于初始Transform状态
	FTransform FinalTransform = CachedInitialTransform;
	
	// 程序移动的最终位置
	FVector ProgramMoveLocation = GetProgramMoveLocation(CurrentAnimationData.Duration); 

	// 关键帧的最终Transform
	FTransform KeyFrameResult = FTransform::Identity;
	if (CurrentAnimationData.KeyFrames.Num() > 0)
	{
		KeyFrameResult = CurrentAnimationData.KeyFrames.Last().Transform;
	}
	
	// 根据bRelativeToStartTransform决定如何叠加
	if (CurrentAnimationData.bRelativeToStartTransform)
	{
		// 关键帧相对于程序移动结果
		FinalTransform.SetLocation(ProgramMoveLocation + KeyFrameResult.GetLocation());
		FinalTransform.SetRotation(FinalTransform.GetRotation() * KeyFrameResult.GetRotation());
		FinalTransform.SetScale3D(FinalTransform.GetScale3D() * KeyFrameResult.GetScale3D());
	}
	else
	{
		// 程序移动控制位置，关键帧控制旋转缩放（如果非Identity）
		FinalTransform.SetLocation(ProgramMoveLocation);
		if (!KeyFrameResult.GetRotation().IsIdentity())
		{
			FinalTransform.SetRotation(KeyFrameResult.GetRotation());
		}
		if (!KeyFrameResult.GetScale3D().Equals(FVector::OneVector))
		{
			FinalTransform.SetScale3D(KeyFrameResult.GetScale3D());
		}
	}
	return FinalTransform;
}

FVector UProgramAniComponent::GetProgramMoveLocation(float ElapsedTime) const
{
	float ProgramMoveAlpha = GetProgramMoveAlpha(ElapsedTime);
	// 检查是否有程序移动曲线
	bool bHasProgramMoveCurve = CurrentAnimationData.ProgramMoveCurve.GetRichCurveConst()->Keys.Num() > 0;
	if (bHasProgramMoveCurve)
	{
		float CurveValue = CurrentAnimationData.ProgramMoveCurve.GetRichCurveConst()->Eval(ProgramMoveAlpha);
		return  FMath::Lerp(CachedStartLocation, CachedEndLocation, CurveValue);
	}

	// 没有曲线时，线性插值程序移动
	return FMath::Lerp(CachedStartLocation, CachedEndLocation, ProgramMoveAlpha);
}

void UProgramAniComponent::UpdateAnimation()
{
	if (!bIsPlayingAnimation)
	{
		return;
	}
	
	// 计算经过时间
	float ElapsedTime = GetWorld()->GetTimeSeconds() - AnimationStartTime;
	
	// 检查动画是否结束
	bool bAnimationFinished = ElapsedTime >= CurrentAnimationData.Duration;

	if (bAnimationFinished)
	{
		// 播放模式处理
		switch (CurrentAnimationData.PlayMode)
		{
			case EProgramAnimationPlayMode::Once:
				{
					// 设置到最终状态并停止
					FTransform FinalTransform;
					if (bIsAnchorMode)
					{
						FinalTransform = CalculateAnchorModeTransform(CurrentAnimationData.Duration);
					}
					else
					{
						FinalTransform = CalculateFinalTransform();
					}
					
					USceneComponent* TargetComponent = GetTargetTransformComponent();
					if (bIsRelativeToParent)
					{
						TargetComponent->SetRelativeTransform(FinalTransform);
					}
					else
					{
						TargetComponent->SetWorldTransform(FinalTransform);
					}
				
					StopAnimation();
				}
				return;
			case EProgramAnimationPlayMode::Loop:
			// 重置开始时间，继续循环
			AnimationStartTime = GetWorld()->GetTimeSeconds();
			ElapsedTime = 0.0f;
			break;
		}
	}

	FTransform FinalTransform;
	
	// 检查是否使用锚点模式
	if (bIsAnchorMode)
	{
		// 锚点模式：直接使用锚点计算Transform
		FinalTransform = CalculateAnchorModeTransform(ElapsedTime);
	}
	else
	{
		// 传统模式：程序移动 + 关键帧
		// 计算程序移动部分（仅位置）
		FVector ProgramMoveLocation = GetProgramMoveLocation(ElapsedTime);

		// 计算关键帧变换
		FTransform KeyFrameTransform = GetKeyFrameTransform(ElapsedTime, CurrentAnimationData);

		// 基于初始Transform创建最终Transform
		FinalTransform = CachedInitialTransform;
		if (CurrentAnimationData.bRelativeToStartTransform)
		{
			// 关键帧相对于程序移动结果
			FinalTransform.SetLocation(ProgramMoveLocation + KeyFrameTransform.GetLocation());
			FinalTransform.SetRotation(FinalTransform.GetRotation() * KeyFrameTransform.GetRotation());
			FinalTransform.SetScale3D(FinalTransform.GetScale3D() * KeyFrameTransform.GetScale3D());
		}
		else
		{
			// 程序移动控制位置，关键帧控制旋转和缩放
			FinalTransform.SetLocation(ProgramMoveLocation);
			if (!KeyFrameTransform.GetRotation().IsIdentity())
			{
				FinalTransform.SetRotation(KeyFrameTransform.GetRotation());
			}
			if (!KeyFrameTransform.GetScale3D().Equals(FVector::OneVector))
			{
				FinalTransform.SetScale3D(KeyFrameTransform.GetScale3D());
			}
		}
	}

	// 应用最终Transform
	USceneComponent* TargetComponent = GetTargetTransformComponent();
	if (bIsRelativeToParent)
	{
		TargetComponent->SetRelativeTransform(FinalTransform);
	}
	else
	{
		TargetComponent->SetWorldTransform(FinalTransform);
	}
}

FVector UProgramAniComponent::SimulateStartTransform(const FProgramAnimationData& AnimationData,
	const FVector& DesiredEndLocation,
	const FVector& CurrentPosition,
	float CurrentProgressInNewAnim) const
{
	// 1. 计算KeyFrame在当前进度的偏移
	float ElapsedTime = CurrentProgressInNewAnim * AnimationData.Duration;
	
	// 直接使用带AnimationData参数的重载函数
	FTransform KeyFrameTransform = GetKeyFrameTransform(ElapsedTime, AnimationData);
	FVector KeyFrameOffset = KeyFrameTransform.GetLocation();
	
	// 2. 计算程序移动的进度和曲线值
	float ProgramMoveAlpha;
	if (AnimationData.ProgramMoveDuration <= 0.0f)
	{
		ProgramMoveAlpha = 1.0f;
	}
	else
	{
		// 考虑ProgramMoveStartTime
		if (ElapsedTime < AnimationData.ProgramMoveStartTime)
		{
			ProgramMoveAlpha = 0.0f; // 程序移动还未开始
		}
		else
		{
			float ProgramMoveElapsedTime = ElapsedTime - AnimationData.ProgramMoveStartTime;
			ProgramMoveAlpha = FMath::Clamp(ProgramMoveElapsedTime / AnimationData.ProgramMoveDuration, 0.0f, 1.0f);
		}
	}
	
	float CurveValue;
	if (AnimationData.ProgramMoveCurve.GetRichCurveConst()->Keys.Num() > 0)
	{
		CurveValue = AnimationData.ProgramMoveCurve.GetRichCurveConst()->Eval(ProgramMoveAlpha);
	}
	else
	{
		// 没有曲线时，线性插值
		CurveValue = ProgramMoveAlpha;
	}
	
	// 3. 边界情况处理
	if (FMath::IsNearlyEqual(CurveValue, 1.0f, KINDA_SMALL_NUMBER))
	{
		// 简化处理：假设KeyFrame是相对于当前位置的
		return CurrentPosition - KeyFrameOffset;
	}
	
	// 4. 反推起始位置
	// 公式：CurrentPosition = StartLocation + (EndLocation - StartLocation) * CurveValue + KeyFrameOffset
	// 整理：CurrentPosition = StartLocation * (1 - CurveValue) + EndLocation * CurveValue + KeyFrameOffset
	// 反推：StartLocation = (CurrentPosition - EndLocation * CurveValue - KeyFrameOffset) / (1 - CurveValue)
	FVector StartLocation = (CurrentPosition - DesiredEndLocation * CurveValue - KeyFrameOffset) / (1.0f - CurveValue);
	
	return StartLocation;
}

FVector UProgramAniComponent::GetCurrentAnchorLocation(float ElapsedTime) const
{
	// 遍历缓存的锚点时间段数组，找到包含当前时间的时间段
	for (const FProgramAnimationAnchor& Segment : CachedAnchorSegments)
	{
		if (ElapsedTime >= Segment.StartTime && ElapsedTime <= Segment.EndTime)
		{
			// 缓存的锚点已经是计算好的世界坐标
			return Segment.AnchorLocation;
		}
	}
	
	// 未找到匹配的时间段，使用默认锚点（默认为世界坐标）
	return CurrentAnimationData.DefaultAnchorLocation;
}

FTransform UProgramAniComponent::CalculateAnchorModeTransform(float ElapsedTime) const
{
	// 1. 获取当前时间的锚点位置
	FVector CurrentAnchor = GetCurrentAnchorLocation(ElapsedTime);
	
	// 2. 获取关键帧Transform
	FTransform KeyFrameTransform = GetKeyFrameTransform(ElapsedTime, CurrentAnimationData);
	
	// 3. 计算围绕锚点的变换
	FTransform Result = CachedInitialTransform;
	
	// 计算初始位置相对于锚点的偏移向量
	FVector InitialOffset = CachedInitialTransform.GetLocation() - CurrentAnchor;
	
	// 对偏移向量应用关键帧旋转变换
	FVector RotatedOffset = KeyFrameTransform.GetRotation().RotateVector(InitialOffset);
	
	// 应用关键帧的位置偏移
	FVector FinalOffset = RotatedOffset + KeyFrameTransform.GetLocation();
	
	// 最终位置 = 锚点 + 变换后的偏移向量
	Result.SetLocation(CurrentAnchor + FinalOffset);
	
	// 旋转：初始旋转 * 关键帧旋转
	Result.SetRotation(CachedInitialTransform.GetRotation() * KeyFrameTransform.GetRotation());
	
	// 缩放：不考虑锚点，直接应用关键帧缩放
	Result.SetScale3D(CachedInitialTransform.GetScale3D() * KeyFrameTransform.GetScale3D());
	
	return Result;
}

void UProgramAniComponent::CacheAnchorLocations()
{
	if (!bIsAnchorMode)
	{
		return;
	}

	// 获取目标组件初始位置（动画开始时的位置）
	USceneComponent* TargetComponent = GetTargetTransformComponent();
	FVector InitialActorLocation;
	if (bIsRelativeToParent)
	{
		InitialActorLocation = TargetComponent->GetRelativeLocation();
	}
	else
	{
		InitialActorLocation = TargetComponent->GetComponentLocation();
	}

	// 清空并重新构建缓存的锚点时间段
	CachedAnchorSegments.Empty();
	CachedAnchorSegments.Reserve(CurrentAnimationData.AnchorTimeSegments.Num());

	for (const FProgramAnimationAnchor& OriginalAnchor : CurrentAnimationData.AnchorTimeSegments)
	{
		FProgramAnimationAnchor CachedAnchor = OriginalAnchor;
		
		if (OriginalAnchor.bRelativeToSelf)
		{
			// 相对锚点：基于初始Actor位置计算实际世界坐标
			CachedAnchor.AnchorLocation = InitialActorLocation + OriginalAnchor.AnchorLocation;
			CachedAnchor.bRelativeToSelf = false; // 缓存后变为世界坐标
		}
		// 世界坐标锚点直接保持不变

		CachedAnchorSegments.Add(CachedAnchor);
	}
}

USceneComponent* UProgramAniComponent::GetTargetTransformComponent() const
{
	return OwnerSceneComponent.IsValid() ? OwnerSceneComponent.Get() : GetOwner()->GetRootComponent();
}