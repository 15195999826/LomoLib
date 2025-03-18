// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RapidIntPayloadBtn.h"
#include "RapidIntPayloadSwitchBtn.generated.h"


/*
 * Struct : UI Item
 */
/**
 * 携带了一个Int返回值， 并且存在是否激活状态的按钮
 */
UCLASS()
class LOMOLIB_API URapidIntPayloadSwitchBtn : public URapidIntPayloadBtn
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSwitchButtonClicked, URapidIntPayloadSwitchBtn*, Btn, int32, OutIntPayload, bool, IsActive);

public:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintAssignable)
	FOnSwitchButtonClicked OnGroupButtonActive;

	void NativeSetActive(bool bInActive);
	
protected:
	// 选中状态
	UPROPERTY(BlueprintReadWrite)
	bool bActive = false;

	UFUNCTION()
	void NativeOnBtnClickedHandler(int32 OutIntPayload);
};
