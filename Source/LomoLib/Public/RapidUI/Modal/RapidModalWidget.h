// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RapidModalWidget.generated.h"

UENUM(BlueprintType)
enum class ERapidModalInputType : uint8
{
	None,
	InputBox UMETA(DisplayName="输入框"),
	ComboBox UMETA(DisplayName="下拉框"),
	OptionBox UMETA(DisplayName="选项框"),
};


USTRUCT(BlueprintType)
struct FRapidModalConfig
{
	GENERATED_BODY()

	FRapidModalConfig(){}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="消息内容"))
	FText InMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="可否取消"))
	bool HasCancelButton = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="是否有输入"))
	ERapidModalInputType InputType = ERapidModalInputType::None;
};

UENUM(BlueprintType)
enum class ERapidModalCloseResult : uint8
{
	Unknown,
	Confirm,
	Cancel,
	Custom
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRapidModalEventSignature, URapidModalWidget*, UserWidget, ERapidModalCloseResult, Result, const FString&, StrPayload);

/**
 * 使用方式： 继承该类实现蓝图， Setup函数按需实现（可不实现）， PerformClose必须设置调用点
 */
UCLASS()
class LOMOLIB_API URapidModalWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PerformClose(ERapidModalCloseResult Result, const FString& StrPayload = TEXT(""));

	UFUNCTION(BlueprintImplementableEvent)
	void Setup(const FRapidModalConfig& InMessage);
	
	/* Delegate part */
	UPROPERTY(BlueprintAssignable, Category = "LCC|EventDispatchers")
	FRapidModalEventSignature OnClose;
};
