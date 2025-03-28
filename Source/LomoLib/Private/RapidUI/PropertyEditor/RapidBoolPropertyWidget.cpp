// Fill out your copyright notice in the Description page of Project Settings.

#include "RapidUI/PropertyEditor/RapidBoolPropertyWidget.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "UObject/UnrealType.h"
void URapidBoolPropertyWidget::InitializePropertyWidget(UObject* InObject, FProperty* InProperty, void* InValuePtr)
{
    Super::InitializePropertyWidget(InObject, InProperty, InValuePtr);
    
    // 绑定复选框事件
    if (ValueCheckBox)
    {
        ValueCheckBox->OnCheckStateChanged.AddDynamic(this, &URapidBoolPropertyWidget::HandleCheckStateChanged);
    }
    
    // 设置属性名称
    if (PropertyNameText)
    {
        PropertyNameText->SetText(PropertyDisplayName);
    }
    
    // 更新初始值
    UpdateValue();
}

void URapidBoolPropertyWidget::UpdateValue_Implementation()
{
    if (Property && ValuePtr)
    {
        FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property);
        if (BoolProperty)
        {
            bCurrentValue = BoolProperty->GetPropertyValue(ValuePtr);
            
            if (ValueCheckBox)
            {
                ValueCheckBox->SetIsChecked(bCurrentValue);
            }
        }
    }
}

void URapidBoolPropertyWidget::SetValue(bool InValue)
{
    if (Property && ValuePtr && TargetObject)
    {
        FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property);
        if (BoolProperty)
        {
            BoolProperty->SetPropertyValue(ValuePtr, InValue);
            bCurrentValue = InValue;
            
            if (ValueCheckBox && ValueCheckBox->IsChecked() != InValue)
            {
                ValueCheckBox->SetIsChecked(InValue);
            }
            
            NotifyPropertyValueChanged();
        }
    }
}

bool URapidBoolPropertyWidget::GetValue() const
{
    return bCurrentValue;
}

void URapidBoolPropertyWidget::HandleCheckStateChanged(bool bIsChecked)
{
    if (bCurrentValue != bIsChecked)
    {
        SetValue(bIsChecked);
    }
} 