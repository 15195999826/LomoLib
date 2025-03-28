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
    Super::InitializePropertyWidget(InObject, InProperty, InValuePtr);
    
    // 绑定SpinBox事件
    if (ValueSpinBox)
    {
        ValueSpinBox->OnValueChanged.AddDynamic(this, &URapidIntPropertyWidget::HandleValueChanged);
        
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
    
    // 设置属性名称
    if (PropertyNameText)
    {
        PropertyNameText->SetText(PropertyDisplayName);
    }
    
    // 更新初始值
    UpdateValue();
}

void URapidIntPropertyWidget::UpdateValue_Implementation()
{
    if (Property && ValuePtr)
    {
        FIntProperty* IntProperty = CastField<FIntProperty>(Property);
        if (IntProperty)
        {
            CurrentValue = IntProperty->GetPropertyValue(ValuePtr);
            
            if (ValueSpinBox)
            {
                ValueSpinBox->SetValue(CurrentValue);
            }
        }
    }
}

void URapidIntPropertyWidget::SetValue(int32 InValue)
{
    if (Property && ValuePtr && TargetObject)
    {
        FIntProperty* IntProperty = CastField<FIntProperty>(Property);
        if (IntProperty)
        {
            IntProperty->SetPropertyValue(ValuePtr, InValue);
            CurrentValue = InValue;
            
            if (ValueSpinBox && FMath::RoundToInt(ValueSpinBox->GetValue()) != InValue)
            {
                ValueSpinBox->SetValue(InValue);
            }
            
            NotifyPropertyValueChanged();
        }
    }
}

int32 URapidIntPropertyWidget::GetValue() const
{
    return CurrentValue;
}

void URapidIntPropertyWidget::HandleValueChanged(float NewValue)
{
    int32 NewIntValue = FMath::RoundToInt(NewValue);
    if (CurrentValue != NewIntValue)
    {
        SetValue(NewIntValue);
    }
}