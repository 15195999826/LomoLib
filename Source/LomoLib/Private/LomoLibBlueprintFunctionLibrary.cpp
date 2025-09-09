// Fill out your copyright notice in the Description page of Project Settings.


#include "LomoLibBlueprintFunctionLibrary.h"

#include "EmptyActor.h"
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
