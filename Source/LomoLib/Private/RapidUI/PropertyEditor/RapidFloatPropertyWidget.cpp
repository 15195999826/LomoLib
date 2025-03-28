// Fill out your copyright notice in the Description page of Project Settings.

#include "RapidUI/PropertyEditor/RapidFloatPropertyWidget.h"
#include "Components/SpinBox.h"
#include "Components/TextBlock.h"
#include "UObject/UnrealType.h"

URapidFloatPropertyWidget::URapidFloatPropertyWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , CurrentValue(0.0f)
{
}

void URapidFloatPropertyWidget::InitializePropertyWidget(UObject* InObject, FProperty* InProperty, void* InValuePtr)
{
    Super::InitializePropertyWidget(InObject, InProperty, InValuePtr);
    
    // 绑定SpinBox事件
    if (ValueSpinBox)
    {
        ValueSpinBox->OnValueChanged.AddDynamic(this, &URapidFloatPropertyWidget::HandleValueChanged);
        
        // 设置SpinBox的默认属性
        ValueSpinBox->SetMinValue(TNumericLimits<float>::Lowest());
        ValueSpinBox->SetMaxValue(TNumericLimits<float>::Max());
        ValueSpinBox->SetDelta(0.1f);
        
        // 从元数据中读取最小最大值
        if (Property && Property->HasMetaData(TEXT("ClampMin")))
        {
            float MinValue = FCString::Atof(*Property->GetMetaData(TEXT("ClampMin")));
            ValueSpinBox->SetMinValue(MinValue);
        }
        
        if (Property && Property->HasMetaData(TEXT("ClampMax")))
        {
            float MaxValue = FCString::Atof(*Property->GetMetaData(TEXT("ClampMax")));
            ValueSpinBox->SetMaxValue(MaxValue);
        }
    }
    
    // 设置属性名称
    if (PropertyNameText)
    {
        PropertyNameText->SetText(PropertyDisplayName);
    }
    
    // 更新初始值
    UpdateValue();
}

void URapidFloatPropertyWidget::UpdateValue_Implementation()
{
    if (Property && ValuePtr)
    {
        FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property);
        if (FloatProperty)
        {
            CurrentValue = FloatProperty->GetPropertyValue(ValuePtr);
            
            if (ValueSpinBox)
            {
                ValueSpinBox->SetValue(CurrentValue);
            }
        }
    }
}

void URapidFloatPropertyWidget::SetValue(float InValue)
{
    if (Property && ValuePtr && TargetObject)
    {
        FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property);
        if (FloatProperty)
        {
            FloatProperty->SetPropertyValue(ValuePtr, InValue);
            CurrentValue = InValue;
            
            if (ValueSpinBox && ValueSpinBox->GetValue() != InValue)
            {
                ValueSpinBox->SetValue(InValue);
            }
            
            NotifyPropertyValueChanged();
        }
    }
}

float URapidFloatPropertyWidget::GetValue() const
{
    return CurrentValue;
}

void URapidFloatPropertyWidget::HandleValueChanged(float NewValue)
{
    if (CurrentValue != NewValue)
    {
        SetValue(NewValue);
    }
} 