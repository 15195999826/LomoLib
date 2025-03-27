// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RapidButtonGroup.generated.h"

USTRUCT(BlueprintType)
struct FRapidGroupBtnConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 IntPayload = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CustomViewFlag;
};

class URapidIntPayloadSwitchBtn;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangeActiveButton, int32, InIntPayload);
/**
 * 按钮组，用于管理一组按钮， 内部的Button必须是ULomoGroupButton的子类
 */
UCLASS()
class LOMOLIB_API URapidButtonGroup : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnChangeActiveButton OnChangeActiveButton;

protected:
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UPanelWidget> ButtonContainer;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<URapidIntPayloadSwitchBtn> ActiveButton;
public:
	UFUNCTION(BlueprintCallable)
	void ClearGroup();
	
	UFUNCTION(BlueprintCallable)
	void PcgButtonGroup(TSubclassOf<URapidIntPayloadSwitchBtn> BtnClass, TArray<FRapidGroupBtnConfig> InMenuConfig);

	UFUNCTION(BlueprintCallable)
	URapidIntPayloadSwitchBtn* AppendPcgButton(TSubclassOf<URapidIntPayloadSwitchBtn> BtnClass, const FRapidGroupBtnConfig& InBtnConfig);
	
	UFUNCTION(BlueprintCallable)
	void DeActiveAll();

	/**
	 * 会
	 * @param InIndex
	 * @param bNotify 是否触发OnChangeActiveButton
	 */
	UFUNCTION(BlueprintCallable)
	void SetDefaultActiveButton(int32 InIndex, bool bNotify = true);

protected:
	UFUNCTION(BlueprintCallable)
	void BtnGroupNativePreConstruct(UPanelWidget* InButtonContainer);

	/**
	 * 方便在蓝图中自定义按钮的样式、布局
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void CustomProgramBtnStyle(URapidIntPayloadSwitchBtn* InBtn);

	UFUNCTION()
	void OnButtonActive(URapidIntPayloadSwitchBtn* InGroupButton, int32 InIntPayload, bool IsActive);

private:
	bool bAbandonNotifyOneTime{false};
};



