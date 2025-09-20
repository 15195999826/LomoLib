// Fill out your copyright notice in the Description page of Project Settings.


#include "LomoLibBlueprintFunctionLibrary.h"

#include "EmptyActor.h"
#include "WaitGroupManager.h"
#include "Framework/Application/NavigationConfig.h"
#include "CancellableDelayAction.h"
#include "LomoLib.h"
#include "Engine/World.h"
#include "Settings/ProgramAnimationSettings.h"
#if WITH_EDITOR
#include "UnrealEd.h"
#endif

void ULomoLibBlueprintFunctionLibrary::EnableUINav()
{
	FNavigationConfig& NavConfig = *FSlateApplication::Get().GetNavigationConfig();
	NavConfig.bTabNavigation = true;
}

void ULomoLibBlueprintFunctionLibrary::DisableUINav()
{
	FNavigationConfig& NavConfig = *FSlateApplication::Get().GetNavigationConfig();
	NavConfig.bTabNavigation = false;
}

void ULomoLibBlueprintFunctionLibrary::PausePIE()
{
#if WITH_EDITOR
	GUnrealEd->PlayWorld->bDebugPauseExecution = true;
#endif
}

FVector ULomoLibBlueprintFunctionLibrary::WorldToRelativeLocation(UObject* WorldContext, const FVector& InWorldLocation,
	AActor* RelativeToActor)
{
	// 创建一个空Actor
	auto TempedActor = WorldContext->GetWorld()->SpawnActor<AEmptyActor>(AEmptyActor::StaticClass(), InWorldLocation, FRotator::ZeroRotator);
	TempedActor->AttachToActor(RelativeToActor, FAttachmentTransformRules::KeepWorldTransform);
	auto RelativeLocation = TempedActor->GetRootComponent()->GetRelativeLocation();
	TempedActor->Destroy();
	return RelativeLocation;
}


void ULomoLibBlueprintFunctionLibrary::DoneWaitGroupOnce(UObject* WorldContextObject, const int32 InWaitGroupID,
	const FName InDebugText)
{
	if (InWaitGroupID == INDEX_NONE)
	{
		return;
	}
	
	auto WaitGroupManager = WorldContextObject->GetWorld()->GetSubsystem<UWaitGroupManager>();
	auto WG = WaitGroupManager->FindWaitGroup(InWaitGroupID);
	if (WG)
	{
		WG->Done(InDebugText);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[DoneOnceWaitGroup]WaitGroupID: %d 不存在, Debug: %s"), InWaitGroupID,
			   *InDebugText.ToString());
	}
}

FFaceRotation ULomoLibBlueprintFunctionLibrary::GetFaceRotation(const FVector& InTargetLocation,
	const FVector& InSourceLocation)
{
	// 计算从源位置到目标位置的方向
	FVector Direction = FVector(InTargetLocation.X - InSourceLocation.X, 
		InTargetLocation.Y - InSourceLocation.Y, 0.0f);
	
	if (Direction.IsNearlyZero())
	{
		return FFaceRotation(false, FRotator::ZeroRotator);
	}
	
	FRotator FaceRotation = Direction.Rotation();
	return FFaceRotation(true, FaceRotation);
}
void ULomoLibBlueprintFunctionLibrary::DelayWithCancel(const UObject* WorldContextObject, float Duration, FLatentActionInfo LatentInfo, TScriptInterface<ICancellable> CancellableContext)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		
		// Check if an action with the same UUID already exists, if so don't create a new one
		if (LatentActionManager.FindExistingAction<FCancellableDelayAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
		{
			// Create and register the new cancellable delay action
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, 
				new FCancellableDelayAction(Duration, LatentInfo, CancellableContext));
		}
	}
}

// Template specialization for float
template<>
float ULomoLibBlueprintFunctionLibrary::GetUserConfig<float>(const FString& InKey, const float& InDefaultValue)
{
	float Ret;
	if (GConfig->GetFloat(TEXT("Project"), *InKey, Ret, GGameUserSettingsIni))
	{
		return Ret;
	}
	return InDefaultValue;
}

// Template specialization for int32
template<>
int32 ULomoLibBlueprintFunctionLibrary::GetUserConfig<int32>(const FString& InKey, const int32& InDefaultValue)
{
	int32 Ret;
	if (GConfig->GetInt(TEXT("Project"), *InKey, Ret, GGameUserSettingsIni))
	{
		return Ret;
	}
	return InDefaultValue;
}

// Template specialization for bool
template<>
bool ULomoLibBlueprintFunctionLibrary::GetUserConfig<bool>(const FString& InKey, const bool& InDefaultValue)
{
	bool Ret;
	if (GConfig->GetBool(TEXT("Project"), *InKey, Ret, GGameUserSettingsIni))
	{
		return Ret;
	}
	return InDefaultValue;
}

// Template specialization for FString
template<>
FString ULomoLibBlueprintFunctionLibrary::GetUserConfig<FString>(const FString& InKey, const FString& InDefaultValue)
{
	FString Ret;
	if (GConfig->GetString(TEXT("Project"), *InKey, Ret, GGameUserSettingsIni))
	{
		return Ret;
	}
	return InDefaultValue;
}

const FProgramAnimationData& ULomoLibBlueprintFunctionLibrary::GetProgramAnimationData(const FName& InAniName)
{
	auto ProgramSettings = GetDefault<UProgramAnimationSettings>();
	if (ProgramSettings->ProgramAnimationAssets.Contains(InAniName))
	{
		auto AnimationDataAsset = ProgramSettings->ProgramAnimationAssets[InAniName].LoadSynchronous();
		return AnimationDataAsset->AnimationData;
	}

	if (ProgramSettings->GeneratedReverseAnimations.Contains(InAniName))
	{
		return ProgramSettings->GeneratedReverseAnimations[InAniName];
	}
	
	static FProgramAnimationData EmptyData;
	UE_LOG(LogLomoLib, Error, TEXT("[GetProgramAnimationData] 未找到对应的动画数据: %s"), *InAniName.ToString());
	return EmptyData;
}

FProgramAnimationData ULomoLibBlueprintFunctionLibrary::RotateProgramAnimationData(
	const FName& InAniName, const FVector& InDirection, const FVector& BaseAnimationDirection)
{
	// Copy
	FProgramAnimationData ResultData = GetProgramAnimationData(InAniName);

	// 默认方向和目标方向
	FVector TargetDirection = InDirection.GetSafeNormal();
	
	// 如果目标方向与默认方向相同，直接返回原始数据
	if (TargetDirection.Equals(BaseAnimationDirection, 0.01f))
	{
		return ResultData;
	}
	
	// 计算从默认方向到目标方向的Z轴旋转角度
	float RotationAngle = FMath::Atan2(TargetDirection.X, -TargetDirection.Y); // 注意：默认方向是(0,-1,0)
	FQuat ZRotation = FQuat(FVector::UpVector, RotationAngle);
	
	// 特殊情况：对于相对的方向（180度），使用简单的Y轴镜像
	float DirectionFactor = FVector::DotProduct(BaseAnimationDirection, TargetDirection);
	bool bUseSimpleMirror = FMath::Abs(DirectionFactor + 1.0f) < 0.01f; // 点积接近-1时为相对方向
	
	
	// 对锚点位置进行变换
	for (FProgramAnimationAnchor& Anchor : ResultData.AnchorTimeSegments)
	{
		FVector OriginalLocation = Anchor.AnchorLocation;
		
		if (bUseSimpleMirror)
		{
			// 相对方向使用Y轴镜像
			Anchor.AnchorLocation.Y = -Anchor.AnchorLocation.Y;
		}
		else if (!FMath::IsNearlyZero(RotationAngle))
		{
			// 其他方向使用Z轴旋转
			Anchor.AnchorLocation = ZRotation.RotateVector(OriginalLocation);
		}
		
	}
	
	// 对默认锚点也进行相同处理
	if (!ResultData.DefaultAnchorLocation.IsZero())
	{
		FVector OriginalDefaultAnchor = ResultData.DefaultAnchorLocation;
		
		if (bUseSimpleMirror)
		{
			ResultData.DefaultAnchorLocation.Y = -ResultData.DefaultAnchorLocation.Y;
		}
		else if (!FMath::IsNearlyZero(RotationAngle))
		{
			ResultData.DefaultAnchorLocation = ZRotation.RotateVector(OriginalDefaultAnchor);
		}
	}
	
	// 对所有关键帧的旋转和位移进行变换
	for (FProgramAnimationKeyFrame& KeyFrame : ResultData.KeyFrames)
	{
		FQuat OriginalRotation = KeyFrame.Transform.GetRotation();
		FVector OriginalLocation = KeyFrame.Transform.GetLocation();
		
		if (!OriginalRotation.IsIdentity())
		{
			if (bUseSimpleMirror)
			{
				// 相对方向使用X分量取负
				FQuat NewRotation = FQuat(-OriginalRotation.X, OriginalRotation.Y, OriginalRotation.Z, OriginalRotation.W);
				KeyFrame.Transform.SetRotation(NewRotation);
			}
			else if (!FMath::IsNearlyZero(RotationAngle))
			{
				// 其他方向使用旋转轴变换
				// 获取原始旋转的轴和角度
				FVector OriginalAxis;
				float OriginalAngle;
				OriginalRotation.ToAxisAndAngle(OriginalAxis, OriginalAngle);
				
				// 将原始旋转轴也进行Z轴旋转变换
				FVector NewRotationAxis = ZRotation.RotateVector(OriginalAxis);
				
				// 创建绕新轴的旋转
				FQuat NewRotation = FQuat(NewRotationAxis, OriginalAngle);
				KeyFrame.Transform.SetRotation(NewRotation);
				
			}
		}
		
		// 对关键帧位移进行变换（与旋转使用相同逻辑）
		if (!OriginalLocation.IsZero())
		{
			if (bUseSimpleMirror)
			{
				// 相对方向使用Y轴镜像
				FVector NewLocation = OriginalLocation;
				NewLocation.Y = -NewLocation.Y;
				KeyFrame.Transform.SetLocation(NewLocation);
			}
			else if (!FMath::IsNearlyZero(RotationAngle))
			{
				// 其他方向使用Z轴旋转
				FVector NewLocation = ZRotation.RotateVector(OriginalLocation);
				KeyFrame.Transform.SetLocation(NewLocation);
			}
		}
		
		// 忽略关键帧缩放（按用户要求）
	}
	
	// 更新动画名称以区分不同方向
	FString DirectionString = FString::Printf(TEXT("%s_Dir_%.1f_%.1f_%.1f"), *InAniName.ToString(),
		TargetDirection.X, TargetDirection.Y, TargetDirection.Z);
	ResultData.AnimationName = FName(DirectionString);
	
	return ResultData;
}
