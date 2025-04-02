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

// 添加辅助方法用于设置SpinBox属性
void URapidFloatPropertyWidget::SetupSpinBoxFromProperty()
{
    // 设置SpinBox的默认属性
    ValueSpinBox->SetMinValue(TNumericLimits<float>::Lowest());
    ValueSpinBox->SetMaxValue(TNumericLimits<float>::Max());
    ValueSpinBox->SetDelta(0.1f);
    
    // 从元数据中读取最小最大值
    if (Property->HasMetaData(TEXT("ClampMin")))
    {
        ValueSpinBox->SetMinValue(FCString::Atof(*Property->GetMetaData(TEXT("ClampMin"))));
    }
    
    if (Property->HasMetaData(TEXT("ClampMax")))
    {
        ValueSpinBox->SetMaxValue(FCString::Atof(*Property->GetMetaData(TEXT("ClampMax"))));
    }
}

bool URapidFloatPropertyWidget::InitializePropertyWidget(UObject* InObject, FProperty* InProperty, const FName& InPropertyName)
{
    // 调用父类的初始化方法
    if (!Super::InitializePropertyWidget(InObject, InProperty, InPropertyName))
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidFloatPropertyWidget初始化失败: 父类初始化返回false"));
        return false;
    }

    // 参数校验
    if (!InObject || !InProperty)
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidFloatPropertyWidget初始化失败: 对象或属性为空"));
        return false;
    }
    
    // 确保属性类型正确
    if (!InProperty->IsA<FFloatProperty>())
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidFloatPropertyWidget初始化失败: 属性类型不是Float"));
        return false;
    }
    
    // 设置属性名称文本
    PropertyNameText->SetText(PropertyDisplayName);
    
    // 绑定SpinBox事件
    ValueSpinBox->OnValueChanged.AddDynamic(this, &URapidFloatPropertyWidget::HandleValueChanged);
    
    // 设置SpinBox属性
    SetupSpinBoxFromProperty();
    
    // 更新初始值
    UpdateValue();
    return true;
}

void URapidFloatPropertyWidget::UpdateValue_Implementation()
{
    SafeExecute([&]() {
        // 获取属性值
        FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property);
        if (FloatProperty && TargetObject)
        {
            CurrentValue = FloatProperty->GetPropertyValue_InContainer(TargetObject);
            ValueSpinBox->SetValue(CurrentValue);
            PropertyNameText->SetText(PropertyDisplayName);
        }
    }, TEXT("更新Float属性值失败"));
}

bool URapidFloatPropertyWidget::SetValue(float InValue)
{
    return SafeExecuteWithRet([&]() -> bool {
        if (!TargetObject || !Property)
        {
            UE_LOG(LogTemp, Error, TEXT("设置Float属性值失败: 对象或属性为空"));
            return false;
        }
        
        FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property);
        if (!FloatProperty)
        {
            UE_LOG(LogTemp, Error, TEXT("设置Float属性值失败: 属性类型不是Float"));
            return false;
        }
        
        // 如果值没有变化，直接返回
        if (FMath::IsNearlyEqual(CurrentValue, InValue))
        {
            return false;
        }
        
        // 安全地设置属性值
        FloatProperty->SetPropertyValue_InContainer(TargetObject, InValue);
        CurrentValue = InValue;
        
        // 只更新当值不同时
        if (!FMath::IsNearlyEqual(ValueSpinBox->GetValue(), InValue))
        {
            ValueSpinBox->SetValue(InValue);
        }
        
        // 确保属性名称文本正确显示
        PropertyNameText->SetText(PropertyDisplayName);
        
        // 通知属性值已更改
        NotifyPropertyValueChanged();
        return true;
    }, TEXT("设置Float属性值失败"));
}

float URapidFloatPropertyWidget::GetValue() const
{
    return CurrentValue;
}

void URapidFloatPropertyWidget::HandleValueChanged(float NewValue)
{
    SafeExecute([&, NewValue]() {
        if (!FMath::IsNearlyEqual(CurrentValue, NewValue))
        {
            SetValue(NewValue);
        }
    }, TEXT("处理数值变化失败"));
} 