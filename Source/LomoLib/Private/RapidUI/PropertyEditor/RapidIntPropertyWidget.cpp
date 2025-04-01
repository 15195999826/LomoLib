// Fill out your copyright notice in the Description page of Project Settings.

#include "RapidUI/PropertyEditor/RapidIntPropertyWidget.h"
#include "Components/SpinBox.h"
#include "Components/TextBlock.h"
#include "UObject/UnrealType.h"

URapidIntPropertyWidget::URapidIntPropertyWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , CurrentValue(0)
{
}

void URapidIntPropertyWidget::InitializePropertyWidget(UObject* InObject, FProperty* InProperty, void* InValuePtr)
{
    // 调用基类初始化方法
    Super::InitializePropertyWidget(InObject, InProperty, InValuePtr);
    
    // 检查必要的参数
    if (!InObject || !InProperty || !InValuePtr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid parameters in IntPropertyWidget::InitializePropertyWidget"));
        return;
    }
    
    // 绑定SpinBox事件
    if (ValueSpinBox && ValueSpinBox->IsValidLowLevel())
    {
        ValueSpinBox->OnValueChanged.AddDynamic(this, &URapidIntPropertyWidget::HandleValueChanged);
        
        try
        {
            // 设置SpinBox的默认属性
            ValueSpinBox->SetMinValue(TNumericLimits<int32>::Lowest());
            ValueSpinBox->SetMaxValue(TNumericLimits<int32>::Max());
            ValueSpinBox->SetDelta(1.0f);
            
            // 从元数据中读取最小最大值
            if (Property && Property->HasMetaData(TEXT("ClampMin")))
            {
                int32 MinValue = FCString::Atoi(*Property->GetMetaData(TEXT("ClampMin")));
                ValueSpinBox->SetMinValue(MinValue);
            }
            
            if (Property && Property->HasMetaData(TEXT("ClampMax")))
            {
                int32 MaxValue = FCString::Atoi(*Property->GetMetaData(TEXT("ClampMax")));
                ValueSpinBox->SetMaxValue(MaxValue);
            }
        }
        catch (...)
        {
            UE_LOG(LogTemp, Warning, TEXT("Exception setting SpinBox properties"));
        }
    }
    
    // 设置属性名称
    if (PropertyNameText && PropertyNameText->IsValidLowLevel())
    {
        PropertyNameText->SetText(PropertyDisplayName);
    }
    
    // 更新初始值
    UpdateValue();
}

void URapidIntPropertyWidget::UpdateValue_Implementation()
{
    if (!Property || !ValuePtr || !TargetObject)
    {
        return;
    }
    
    // 确保属性有效
    if (!Property->IsValidLowLevel())
    {
        return;
    }
    
    FIntProperty* IntProperty = CastField<FIntProperty>(Property);
    if (!IntProperty)
    {
        return;
    }
    
    try
    {
        // 安全地获取属性值
        CurrentValue = IntProperty->GetPropertyValue(ValuePtr);
        
        // 更新UI
        if (ValueSpinBox && ValueSpinBox->IsValidLowLevel())
        {
            ValueSpinBox->SetValue(CurrentValue);
        }
    }
    catch (...)
    {
        // 捕获可能的异常
        UE_LOG(LogTemp, Warning, TEXT("UpdateValue exception in RapidIntPropertyWidget for property: %s"), *PropertyName.ToString());
    }
}

void URapidIntPropertyWidget::SetValue(int32 InValue)
{
    if (!Property || !ValuePtr || !TargetObject)
    {
        return;
    }
    
    // 确保属性有效
    if (!Property->IsValidLowLevel())
    {
        return;
    }
    
    FIntProperty* IntProperty = CastField<FIntProperty>(Property);
    if (!IntProperty)
    {
        return;
    }
    
    try
    {
        // 安全地设置属性值
        IntProperty->SetPropertyValue(ValuePtr, InValue);
        CurrentValue = InValue;
        
        // 只有当新值与当前UI值不同时才更新UI
        if (ValueSpinBox && ValueSpinBox->IsValidLowLevel() && 
            FMath::RoundToInt(ValueSpinBox->GetValue()) != InValue)
        {
            ValueSpinBox->SetValue(InValue);
        }
        
        // 通知属性值已更改
        NotifyPropertyValueChanged();
    }
    catch (...)
    {
        // 捕获可能的异常
        UE_LOG(LogTemp, Warning, TEXT("SetValue exception in RapidIntPropertyWidget for property: %s"), *PropertyName.ToString());
    }
}

int32 URapidIntPropertyWidget::GetValue() const
{
    return CurrentValue;
}

void URapidIntPropertyWidget::HandleValueChanged(float NewValue)
{
    if (!Property || !ValuePtr || !TargetObject)
    {
        return;
    }
    
    try
    {
        // 四舍五入为整数
        int32 NewIntValue = FMath::RoundToInt(NewValue);
        
        // 只有在值发生变化时才更新
        if (CurrentValue != NewIntValue)
        {
            SetValue(NewIntValue);
        }
    }
    catch (...)
    {
        // 捕获可能的异常
        UE_LOG(LogTemp, Warning, TEXT("HandleValueChanged exception in RapidIntPropertyWidget for property: %s"), *PropertyName.ToString());
    }
}