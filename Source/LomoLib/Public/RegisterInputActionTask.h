// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "RegisterInputActionTask.generated.h"

class ALomoGeneralPlayerController;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSimpleInputExecuteSignature);

/**
 * 
 */
UCLASS(BlueprintType)
class LOMOLIB_API URegisterInputActionTask : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	// 析构函数
	virtual ~URegisterInputActionTask();

	friend ALomoGeneralPlayerController;
	
	inline static int GlobalTaskID = 0;

	UPROPERTY(BlueprintAssignable)
	FSimpleInputExecuteSignature OnExecute;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		Category = "Seed|UI|Input")
	static URegisterInputActionTask* RegisterInputActionTask(const UObject* WorldContextObject,
	                                                         FKey InKey,
	                                                         TEnumAsByte<EInputEvent> InInputEvent,
	                                                         URegisterInputActionTask*& OutTask);

	
	virtual void Activate() override;
	//~UBlueprintAsyncActionBase interface

	// Allows the Object to use BP_Functions
	virtual class UWorld* GetWorld() const override;

	void EndTask();
	UFUNCTION(BlueprintCallable)
	void ActiveEnd();

protected:
	const UObject* WorldContextObject;
	int TaskID;
	FKey Key;
	TEnumAsByte<EInputEvent> InputEvent;

	FSimpleDelegate Delegate;
	void OnInputAction();
	
};
