// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncWaitForLevelStatus.generated.h"

class ULevelStreamingDynamic;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSimpleLevelAddOutSignature);

/**
 * 
 */
UCLASS()
class LOMOLIB_API UAsyncWaitForLevelStatus : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable,
		meta = (BlueprintInternalUseOnly = "true"),
		Category = "Async")
	static UAsyncWaitForLevelStatus* AsyncWaitForLevelAdded(UObject* WorldContext, ULevelStreamingDynamic* LvInstance, bool InTrueForLoadFalseForUnload = true);

	virtual void Activate() override;
	
	UPROPERTY(BlueprintAssignable)
	FSimpleLevelAddOutSignature OnComplete;

	UPROPERTY(BlueprintAssignable)
	FSimpleLevelAddOutSignature Failed;

protected:
	bool TrueForLoadFalseForUnload;
	
	UPROPERTY()
	TWeakObjectPtr<ULevelStreamingDynamic> LevelInstance;

	void CheckLevelAdded();

	FTimerHandle LevelAddedTimerHandle;

	void BroadcastFail();
	void BroadcastSuccess();

private:
	UPROPERTY()
	TArray<FString> LoadedLevelPackageName;
	
	FDelegateHandle RemoveHandle;
	
	void OnLevelRemoved(ULevel* Level, UWorld* World);
};
