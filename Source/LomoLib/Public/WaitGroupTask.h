// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "WaitGroupTask.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaitGroupGeneralSignature, int32, WaitGroupID);
/**
 * 步任务类，创建一个WaitGroup, 返回WaitGroupID,  在完成全部任务后触发回调
 * 任务完成时自动销毁
 * Todo: 但有时存在无法销毁的情况, 可能需要设置一个超时参数, 但是暂时不管 
 */
UCLASS(Blueprintable)
class LOMOLIB_API UWaitGroupTask : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	virtual UWorld* GetWorld() const override;
public:
	/**
	  * 创建并执行技能表演任务
	  * @param WorldContext
	  * @param InTaskCount
	  * @return 返回创建的任务实例
	  */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", Category = "AsyncTask", DeterminesOutputType="TaskClass", WorldContext = "WorldContext"))
	static UWaitGroupTask* WaitGroupTask(
		UObject* WorldContext,
		int32 InTaskCount = 1
		);

	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable)
	FOnWaitGroupGeneralSignature OnCreate;
	
	/** 任务完成时触发 */
	UPROPERTY(BlueprintAssignable)
	FOnWaitGroupGeneralSignature OnCompleted;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetWaitGroupID() const
	{
		return WaitGroupID;
	}

private:
	int32 WaitGroupID = INDEX_NONE;
	int32 TaskCount = 0;
};
