// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncWaitForLevelStatus.h"

#include "LomoLib.h"
#include "Engine/LevelStreamingDynamic.h"

UAsyncWaitForLevelStatus* UAsyncWaitForLevelStatus::AsyncWaitForLevelAdded(
	UObject* WorldContext, ULevelStreamingDynamic* LvInstance,  bool InTrueForLoadFalseForUnload)
{
	auto Task = NewObject<UAsyncWaitForLevelStatus>(WorldContext);
	Task->TrueForLoadFalseForUnload = InTrueForLoadFalseForUnload;
	Task->LevelInstance = LvInstance;
	Task->RegisterWithGameInstance(WorldContext->GetWorld());
	return Task;
}

void UAsyncWaitForLevelStatus::BroadcastFail()
{
	Failed.Broadcast();
	SetReadyToDestroy();
}

void UAsyncWaitForLevelStatus::BroadcastSuccess()
{
	OnComplete.Broadcast();
	SetReadyToDestroy();
}

void UAsyncWaitForLevelStatus::OnLevelRemoved(ULevel* Level, UWorld* World)
{
	FString LevelPackageName = FPackageName::ObjectPathToPackageName(Level->GetPathName());
	UE_LOG(LogLomoLib, Log, TEXT("Level %s is removed"), *LevelPackageName);
	LoadedLevelPackageName.Remove(LevelPackageName);

	if (LoadedLevelPackageName.Num() == 0)
	{
		FWorldDelegates::LevelRemovedFromWorld.Remove(RemoveHandle);
		BroadcastSuccess();
	}
}

void UAsyncWaitForLevelStatus::Activate()
{
	if (!LevelInstance.IsValid())
	{
		BroadcastFail();
		return;
	}

	if (TrueForLoadFalseForUnload)
	{
		GetWorld()->GetTimerManager().SetTimer(LevelAddedTimerHandle, this, &UAsyncWaitForLevelStatus::CheckLevelAdded, 0.1f, true);
	}
	else
	{
		TArray<ULevelStreaming*> StreamingLevels = LevelInstance->GetLoadedLevel()->GetWorld()->GetStreamingLevels();
		for (ULevelStreaming* StreamingLevel : StreamingLevels)
		{
			if (StreamingLevel->IsLevelLoaded())
			{
				UE_LOG(LogLomoLib, Log, TEXT("Level %s Wait To Unload"), *StreamingLevel->GetWorldAssetPackageName());
				LoadedLevelPackageName.Add(StreamingLevel->GetWorldAssetPackageName());
			}
		}
		RemoveHandle = FWorldDelegates::LevelRemovedFromWorld.AddUObject(
			this, &UAsyncWaitForLevelStatus::OnLevelRemoved);
	}
}

void UAsyncWaitForLevelStatus::CheckLevelAdded()
{
	if (LevelInstance->IsLevelLoaded())
	{
		// 获取LevelInstance中的所有流式关卡
		TArray<ULevelStreaming*> StreamingLevels = LevelInstance->GetLoadedLevel()->GetWorld()->GetStreamingLevels();

		// 遍历所有的流式关卡
		for (ULevelStreaming* StreamingLevel : StreamingLevels)
		{
			// 如果有任何一个流式关卡还没有加载完成，就返回，等待下一次检查
			if (!StreamingLevel->IsLevelLoaded()|| !StreamingLevel->IsLevelVisible())
			{
				UE_LOG(LogLomoLib, Error, TEXT("Level %s is not loaded yet"), *StreamingLevel->GetWorldAssetPackageName());
				return;
			}
			UE_LOG(LogLomoLib, Warning, TEXT("Level %s is loaded yet"), *StreamingLevel->GetWorldAssetPackageName());
		}
		
		BroadcastSuccess();
		GetWorld()->GetTimerManager().ClearTimer(LevelAddedTimerHandle);
	} 
}
