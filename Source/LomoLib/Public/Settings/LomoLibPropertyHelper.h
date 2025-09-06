// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LomoLibPropertyHelper.generated.h"

/**
 * LomoLib属性助手类，为编辑器提供动态选项
 */
UCLASS()
class LOMOLIB_API ULomoLibPropertyHelper : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UFUNCTION()
	static TArray<FName> GetProgramAnimations();
};