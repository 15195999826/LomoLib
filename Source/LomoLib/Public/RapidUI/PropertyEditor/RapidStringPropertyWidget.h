// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RapidUI/PropertyEditor/RapidPropertyWidget.h"
#include "RapidStringPropertyWidget.generated.h"

class UEditableTextBox;
class UTextBlock;

/**
 * 字符串类型的属性控件，可用于FString, FName, FText类型
 */
UCLASS(BlueprintType, Blueprintable)
class LOMOLIB_API URapidStringPropertyWidget : public URapidPropertyWidget
{
    GENERATED_BODY()

public:
    URapidStringPropertyWidget(const FObjectInitializer& ObjectInitializer);
    
    // 重写初始化方法
    virtual void InitializePropertyWidget(UObject* InObject, FProperty* InProperty, void* InValuePtr) override;
    
    // 重写更新值方法
    virtual void UpdateValue_Implementation() override;
    
    // 设置字符串值
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    void SetValue(const FString& InValue);
    
    // 获取字符串值
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    FString GetValue() const;
    
protected:
    // 属性名称显示
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* PropertyNameText;
    
    // 文本编辑框
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UEditableTextBox* ValueTextBox;
    
    // 文本提交时的处理函数
    UFUNCTION()
    void HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
    
    // 文本变化时的处理函数
    UFUNCTION()
    void HandleTextChanged(const FText& Text);
    
private:
    // 当前值
    FString CurrentValue;
    
    // 属性类型
    enum class EPropertyType
    {
        String,
        Name,
        Text
    };
    
    // 当前属性类型
    EPropertyType PropertyType;
}; 