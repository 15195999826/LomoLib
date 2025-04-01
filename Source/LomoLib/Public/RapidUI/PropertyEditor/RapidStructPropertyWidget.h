// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RapidUI/PropertyEditor/RapidPropertyWidget.h"
#include "RapidStructPropertyWidget.generated.h"

class UTextBlock;
class UExpandableArea;
class UVerticalBox;
class UScrollBox;

/**
 * 结构体类型的属性控件
 */
UCLASS(BlueprintType, Blueprintable)
class LOMOLIB_API URapidStructPropertyWidget : public URapidPropertyWidget
{
    GENERATED_BODY()

public:
    URapidStructPropertyWidget(const FObjectInitializer& ObjectInitializer);
    
    // 重写初始化方法
    virtual bool InitializePropertyWidget(UObject* InObject, FProperty* InProperty, const FName& InPropertyName) override;
    
    // 重写更新值方法
    virtual void UpdateValue_Implementation() override;
    
protected:
    // 标题文本
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* TitleText;
    
    // 可展开区域
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UExpandableArea* ExpandableArea;
    
    // 内容垂直框
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UVerticalBox* ContentVerticalBox;
    
    // 处理子属性值改变
    UFUNCTION()
    void HandleChildPropertyValueChanged(UObject* Object, FName InPropertyName, URapidPropertyWidget* PropertyWidget);
    
private:
    // 创建结构体中的所有子属性控件
    void CreateChildProperties();
    
    // 生成的属性控件
    UPROPERTY()
    TArray<URapidPropertyWidget*> ChildPropertyWidgets;
}; 