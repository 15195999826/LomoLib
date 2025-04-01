// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RapidUI/PropertyEditor/RapidPropertyWidget.h"
#include "RapidFloatPropertyWidget.generated.h"

class USpinBox;
class UTextBlock;

/**
 * Float类型的属性控件
 */
UCLASS(BlueprintType, Blueprintable)
class LOMOLIB_API URapidFloatPropertyWidget : public URapidPropertyWidget
{
    GENERATED_BODY()

public:
    URapidFloatPropertyWidget(const FObjectInitializer& ObjectInitializer);

    // 重写初始化方法
    virtual bool InitializePropertyWidget(UObject* InObject, FProperty* InProperty, const FName& InPropertyName) override;

    // 重写更新值方法
    virtual void UpdateValue_Implementation() override;

    // 设置浮点值
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    bool SetValue(float InValue);

    // 获取浮点值
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    float GetValue() const;

protected:
    // 属性名称显示
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* PropertyNameText;

    // 数值编辑框
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    USpinBox* ValueSpinBox;

    // 数值变化时的处理函数
    UFUNCTION()
    void HandleValueChanged(float NewValue);

private:
    // 当前值
    float CurrentValue;
    
    // 设置SpinBox属性的辅助方法
    void SetupSpinBoxFromProperty();
}; 