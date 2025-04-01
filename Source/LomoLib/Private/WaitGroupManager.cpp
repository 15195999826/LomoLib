// Fill out your copyright notice in the Description page of Project Settings.


#include "WaitGroupManager.h"

int32 UWaitGroupManager::EmptyID = -1;

void UWaitGroupManager::BeginDestroy()
{
	CleanupAllWaitGroups();
	Super::BeginDestroy();
}

TTuple<int32, TSharedRef<FLomoWaitGroup>> UWaitGroupManager::CreateWaitGroup(const FName& InWGDebugName)
{
	const int32 CurrentID = NextWaitGroupID++;
	TSharedRef<FLomoWaitGroup> NewWG = MakeShared<FLomoWaitGroup>(CurrentID);
	NewWG->SetName(InWGDebugName);
	
	ActiveWaitGroups.Add(CurrentID, NewWG);

	// 设置完成回调
	NewWG->OnCompleted.AddUObject(this, &UWaitGroupManager::OnWaitGroupCompleted);

	return MakeTuple(CurrentID, NewWG);
}

TSharedPtr<FLomoWaitGroup> UWaitGroupManager::FindWaitGroup(int32 InID)
{
	if (ActiveWaitGroups.Contains(InID))
	{
		return ActiveWaitGroups[InID];
	}
	
	UE_LOG(LogTemp, Warning, TEXT("WaitGroup ID: %d not found"), InID);
	return nullptr;
}

void UWaitGroupManager::CleanupAllWaitGroups()
{
	if (ActiveWaitGroups.Num() == 0)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Cleaning up %d active WaitGroups"), ActiveWaitGroups.Num());

	// 1. 先解绑所有完成回调，防止触发 OnWaitGroupCompleted
	for (auto& Pair : ActiveWaitGroups)
	{
		Pair.Value->OnCompleted.RemoveAll(this);
	}

	// 2. 清理所有任务和Promise，保持 WaitGroup 引用
	for (auto& Pair : ActiveWaitGroups)
	{
		UE_LOG(LogTemp, Warning, TEXT("强制结束 WaitGroup ID: %d, Name: %s"), 
		Pair.Key, *Pair.Value->GetDebugName().ToString());
		// 清除内部状态但保持对象存活
		Pair.Value->bIsCancelled = true;
		Pair.Value->Counter = 1;
		Pair.Value->Done();
	}

	// 3. 最后清空容器（此时回调已解绑，状态已重置）
	ActiveWaitGroups.Empty();
}

void UWaitGroupManager::OnWaitGroupCompleted(int32 InID)
{
	// 查找并移除已完成的 WaitGroup
	if (ActiveWaitGroups.Contains(InID))
	{
		ActiveWaitGroups.Remove(InID);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WaitGroup ID: %d not found"), InID);
	}
}
