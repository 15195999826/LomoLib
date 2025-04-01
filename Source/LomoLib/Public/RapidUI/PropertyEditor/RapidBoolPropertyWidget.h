// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RapidUI/PropertyEditor/RapidPropertyWidget.h"
#include "RapidBoolPropertyWidget.generated.h"

class UCheckBox;
class UTextBlock;

/**
 * Bool类型的属性控件
 */
UCLASS(BlueprintType, Blueprintable)
class LOMOLIB_API URapidBoolPropertyWidget : public URapidPropertyWidget
{
    GENERATED_BODY()

public:
    // 重写初始化方法
    virtual bool InitializePropertyWidget(UObject* InObject, FProperty* InProperty, const FName& InPropertyName) override;
    
    // 重写更新值方法
    virtual void UpdateValue_Implementation() override;
    
    // 设置布尔值
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    bool SetValue(bool InValue);
    
    // 获取布尔值
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    bool GetValue() const;
    
protected:
    // 属性名称显示
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* PropertyNameText;
    
    // 复选框
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UCheckBox* ValueCheckBox;
    
    // 复选框状态变化时的处理函数
    UFUNCTION()
    void HandleCheckStateChanged(bool bIsChecked);
    
private:
    // 当前值
    bool bCurrentValue;
}; 