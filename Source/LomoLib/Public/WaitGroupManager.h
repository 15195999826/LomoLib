// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LomoWaitGroup.h"
#include "Subsystems/WorldSubsystem.h"
#include "WaitGroupManager.generated.h"

/**
 * 
 */
UCLASS()
class LOMOLIB_API UWaitGroupManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static int32 EmptyID;
	// 从管理器获取新的 WaitGroup
	TTuple<int32, TSharedRef<FLomoWaitGroup>> CreateWaitGroup(const FName& InWGDebugName);

	TSharedPtr<FLomoWaitGroup> FindWaitGroup(int32 InID);

	// 清理所有活跃的 WaitGroup
	void CleanupAllWaitGroups();

	// UWorldSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	virtual void Deinitialize() override;

private:
	int32 NextWaitGroupID = 0;
	// 存储所有活跃的 WaitGroup
	TMap<int32, TSharedRef<FLomoWaitGroup>> ActiveWaitGroups;

	// 当 WaitGroup 完成时的回调
	void OnWaitGroupCompleted(int32 InID);
};
