// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedPlayerInput.h"
#include "LomoPlayerInput.generated.h"

/**
 * 
 */
UCLASS()
class LOMOLIB_API ULomoPlayerInput : public UEnhancedPlayerInput
{
	GENERATED_BODY()

public:
	UPROPERTY()
	bool DebugKeyConsume = false;
	// 重写委托评估方法
	virtual void EvaluateInputDelegates(const TArray<UInputComponent*>& InputComponentStack, const float DeltaTime, const bool bGamePaused, const TArray<TPair<FKey, FKeyState*>>& KeysWithEvents) override;

private:
	UPROPERTY()
	TMap<FKey, bool> PreviousConsumedStates;
};
