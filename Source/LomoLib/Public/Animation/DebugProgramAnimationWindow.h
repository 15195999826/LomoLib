// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProgramAnimationTypes.h"
#include "Blueprint/UserWidget.h"
#include "DebugProgramAnimationWindow.generated.h"

class UComboBoxString;
/**
 * 
 */
UCLASS()
class LOMOLIB_API UDebugProgramAnimationWindow : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UComboBoxString> AnimationSelector;

	UFUNCTION(BlueprintCallable)
	void InitWindow();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const FProgramAnimationData& GetAnimationData(const FString& InAniName, bool bReverse);
};
