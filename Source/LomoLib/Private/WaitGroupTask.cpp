// Fill out your copyright notice in the Description page of Project Settings.


#include "WaitGroupTask.h"

#include "WaitGroupManager.h"

UWorld* UWaitGroupTask::GetWorld() const
{
	if (GetOuter() == nullptr)
	{
		return nullptr;
	}

	// Special case for behavior tree nodes in the editor
	if (Cast<UPackage>(GetOuter()) != nullptr)
	{
		// GetOuter should return a UPackage and its Outer is a UWorld
		return Cast<UWorld>(GetOuter()->GetOuter());
	}

	// In all other cases...
	return GetOuter()->GetWorld();
}

UWaitGroupTask* UWaitGroupTask::WaitGroupTask(UObject* WorldContext, int32 InTaskCount)
{
	UWaitGroupTask* Task = NewObject<UWaitGroupTask>(WorldContext);
	Task->TaskCount = InTaskCount;
	return Task;
}

void UWaitGroupTask::Activate()
{
	Super::Activate();
	if (TaskCount <= 0)
	{
		// If the task count is zero or negative, we can complete immediately
		OnCompleted.Broadcast(-1);
		SetReadyToDestroy();
		return;
	}

	auto WaitGroupManager = GetWorld()->GetSubsystem<UWaitGroupManager>();
	auto WGTuple = WaitGroupManager->CreateWaitGroup("WaitGroupTask");
	WaitGroupID = WGTuple.Key;
	WGTuple.Value->Add(TaskCount);

	OnCreate.Broadcast(WaitGroupID);

	WGTuple.Value->Next([this]()
	{
		OnCompleted.Broadcast(WaitGroupID);
		SetReadyToDestroy();
	});
}
