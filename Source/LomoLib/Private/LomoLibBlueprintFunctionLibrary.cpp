// Fill out your copyright notice in the Description page of Project Settings.


#include "LomoLibBlueprintFunctionLibrary.h"

#include "EmptyActor.h"
#include "WaitGroupManager.h"
#include "Framework/Application/NavigationConfig.h"
#include "CancellableDelayAction.h"
#include "Engine/World.h"
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