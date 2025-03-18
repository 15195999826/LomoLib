// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RapidIntPayloadBtn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRapidIntBtnClickedSignature, int32, IntPayload); 
/**
 * 点击后携带一个int返回值的按钮
 */
UCLASS()
class LOMOLIB_API URapidIntPayloadBtn : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnRapidIntBtnClickedSignature OnBtnClicked;

	// 代码侧程序生成按钮时, 绑定Payload, 并且执行蓝图事件， 通过一个简单的String Flag, 传递到蓝图中， 用于创建不同的样式
	void ProgramBindPayload(const int32 InIntPayload, const FString& InCustomViewFlag = TEXT(""))
	{
		IntPayload = InIntPayload;
		CreateViewFromProgram(InCustomViewFlag);
	}

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="Int返回值"))
	int32 IntPayload;

	UFUNCTION(BlueprintImplementableEvent)
	void CreateViewFromProgram(const FString& InCustomViewFlag = TEXT(""));
};
