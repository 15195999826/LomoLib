// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RapidPropertyEditorWidget.generated.h"

class ULomoLibPropertyEditorTestObject;
class UButton;
class URapidPropertyEditor;

/**
 * 演示如何使用RapidPropertyEditor的示例小部件
 */
UCLASS()
class LOMOLIB_API URapidPropertyEditorWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

protected:
    /** 属性编辑器 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    URapidPropertyEditor* PropertyEditor;
    
    /** 刷新按钮 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* RefreshButton;
    
    /** 重置按钮 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* ResetButton;

    /** 测试对象 */
    UPROPERTY()
    ULomoLibPropertyEditorTestObject* TestObject;

    /** 处理刷新按钮点击 */
    UFUNCTION()
    void HandleRefreshButtonClicked();
    
    /** 处理重置按钮点击 */
    UFUNCTION()
    void HandleResetButtonClicked();
    
    // /** 处理属性改变 */
    // UFUNCTION()
    // void HandlePropertyChanged(UObject* Object, FName PropertyName, const FProperty* Property);
}; 