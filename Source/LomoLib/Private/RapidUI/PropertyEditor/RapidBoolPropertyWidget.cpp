// Fill out your copyright notice in the Description page of Project Settings.

#include "RapidUI/PropertyEditor/RapidBoolPropertyWidget.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "UObject/UnrealType.h"

bool URapidBoolPropertyWidget::InitializePropertyWidget(UObject* InObject, FProperty* InProperty, const FName& InPropertyName)
{
    // 调用父类的初始化方法
    if (!Super::InitializePropertyWidget(InObject, InProperty, InPropertyName))
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidBoolPropertyWidget初始化失败: 父类初始化返回false"));
        return false;
    }

    // 参数校验
    if (!InObject || !InProperty)
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidBoolPropertyWidget初始化失败: 对象或属性为空"));
        return false;
    }
    
    // 确保属性类型正确
    if (!InProperty->IsA<FBoolProperty>())
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidBoolPropertyWidget初始化失败: 属性类型不是Bool"));
        return false;
    }
    
    // 设置属性名称文本
    PropertyNameText->SetText(PropertyDisplayName);
    
    // 绑定复选框事件
    ValueCheckBox->OnCheckStateChanged.AddDynamic(this, &URapidBoolPropertyWidget::HandleCheckStateChanged);
    
    // 更新初始值
    UpdateValue();
    return true;
}

void URapidBoolPropertyWidget::UpdateValue_Implementation()
{
    SafeExecute([&]() {
        // 获取属性值
        FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property);
        if (BoolProperty && TargetObject)
        {
            bCurrentValue = BoolProperty->GetPropertyValue_InContainer(TargetObject);
            ValueCheckBox->SetIsChecked(bCurrentValue);
            PropertyNameText->SetText(PropertyDisplayName);
        }
    }, TEXT("更新Bool属性值失败"));
}

bool URapidBoolPropertyWidget::SetValue(bool InValue)
{
    return SafeExecuteWithRet([&]() -> bool {
        if (!TargetObject || !Property)
        {
            UE_LOG(LogTemp, Error, TEXT("设置Bool属性值失败: 对象或属性为空"));
            return false;
        }
        
        FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property);
        if (!BoolProperty)
        {
            UE_LOG(LogTemp, Error, TEXT("设置Bool属性值失败: 属性类型不是Bool"));
            return false;
        }
        
        // 如果值没有变化，直接返回
        if (bCurrentValue == InValue)
        {
            return false;
        }
        
        // 安全地设置属性值
        BoolProperty->SetPropertyValue_InContainer(TargetObject, InValue);
        bCurrentValue = InValue;
        
        // 只更新当值不同时
        if (ValueCheckBox->IsChecked() != InValue)
        {
            ValueCheckBox->SetIsChecked(InValue);
        }
        
        // 确保属性名称文本正确显示
        PropertyNameText->SetText(PropertyDisplayName);
        
        // 通知属性值已更改
        NotifyPropertyValueChanged();
        return true;
    }, TEXT("设置Bool属性值失败"));
}

bool URapidBoolPropertyWidget::GetValue() const
{
    return bCurrentValue;
}

void URapidBoolPropertyWidget::HandleCheckStateChanged(bool bIsChecked)
{
    SafeExecute([&, bIsChecked]() {
        if (bCurrentValue != bIsChecked)
        {
            SetValue(bIsChecked);
        }
    }, TEXT("处理复选框状态变化失败"));
} 