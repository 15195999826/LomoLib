// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RapidUI/PropertyEditor/RapidPropertyWidget.h"
#include "RapidIntPropertyWidget.generated.h"

class USpinBox;
class UTextBlock;

/**
 * 整数类型的属性控件
 */
UCLASS(BlueprintType, Blueprintable)
class LOMOLIB_API URapidIntPropertyWidget : public URapidPropertyWidget
{
    GENERATED_BODY()

public:
    URapidIntPropertyWidget(const FObjectInitializer& ObjectInitializer);
    
    // 重写初始化方法
    virtual bool InitializePropertyWidget(UObject* InObject, FProperty* InProperty, const FName& InPropertyName) override;
    
    // 重写更新值方法
    virtual void UpdateValue_Implementation() override;
    
    // 设置整数值
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    bool SetValue(int32 InValue);
    
    // 获取整数值
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    int32 GetValue() const;
    
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
    int32 CurrentValue;
    
    // 设置SpinBox属性的辅助方法
    void SetupSpinBoxFromProperty();
};