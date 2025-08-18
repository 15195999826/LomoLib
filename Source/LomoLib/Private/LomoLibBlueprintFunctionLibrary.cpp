// Fill out your copyright notice in the Description page of Project Settings.


#include "LomoLibBlueprintFunctionLibrary.h"

#include "EmptyActor.h"
#include "Framework/Application/NavigationConfig.h"
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
