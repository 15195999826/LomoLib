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

// 添加辅助方法用于设置SpinBox属性
void URapidIntPropertyWidget::SetupSpinBoxFromProperty()
{
    // 设置SpinBox的默认属性
    ValueSpinBox->SetMinValue(TNumericLimits<int32>::Lowest());
    ValueSpinBox->SetMaxValue(TNumericLimits<int32>::Max());
    ValueSpinBox->SetDelta(1.0f);
    
    // 从元数据中读取最小最大值
    if (Property->HasMetaData(TEXT("ClampMin")))
    {
        int32 MinValue = FCString::Atoi(*Property->GetMetaData(TEXT("ClampMin")));
        ValueSpinBox->SetMinValue(MinValue);
    }
    
    if (Property->HasMetaData(TEXT("ClampMax")))
    {
        int32 MaxValue = FCString::Atoi(*Property->GetMetaData(TEXT("ClampMax")));
        ValueSpinBox->SetMaxValue(MaxValue);
    }
}

bool URapidIntPropertyWidget::InitializePropertyWidget(UObject* InObject, FProperty* InProperty, const FName& InPropertyName)
{
    // 调用父类的初始化方法
    if (!Super::InitializePropertyWidget(InObject, InProperty, InPropertyName))
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidIntPropertyWidget初始化失败: 父类初始化返回false"));
        return false;
    }

    // 参数校验
    if (!InObject || !InProperty)
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidIntPropertyWidget初始化失败: 对象或属性为空"));
        return false;
    }
    
    // 确保属性类型正确
    if (!InProperty->IsA<FIntProperty>())
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidIntPropertyWidget初始化失败: 属性类型不是Int"));
        return false;
    }
    
    // 设置属性名称文本 - PropertyNameText由BindWidget保证不为空
    PropertyNameText->SetText(PropertyDisplayName);
    
    // 绑定SpinBox事件 - ValueSpinBox由BindWidget保证不为空
    ValueSpinBox->OnValueChanged.AddDynamic(this, &URapidIntPropertyWidget::HandleValueChanged);
    
    // 设置SpinBox属性
    SafeExecute([&]() {
        SetupSpinBoxFromProperty();
    }, TEXT("初始化SpinBox属性失败"));
    
    // 更新初始值
    UpdateValue();
    return true;
}

void URapidIntPropertyWidget::UpdateValue_Implementation()
{
    SafeExecute([&]() {
        // 初始化当前值
        CurrentValue = 0;
        
        FIntProperty* IntProperty = CastField<FIntProperty>(Property);
        if (IntProperty && TargetObject)
        {
            // 安全地获取属性值
            CurrentValue = IntProperty->GetPropertyValue_InContainer(TargetObject);
            
            // 更新UI - ValueSpinBox由BindWidget保证不为空
            ValueSpinBox->SetValue(CurrentValue);
            
            // 确保属性名称文本正确显示 - PropertyNameText由BindWidget保证不为空
            PropertyNameText->SetText(PropertyDisplayName);
        }
    }, TEXT("更新Int属性值失败"));
}

bool URapidIntPropertyWidget::SetValue(int32 InValue)
{
    return SafeExecuteWithRet([&]() -> bool {
        if (!TargetObject || !Property)
        {
            UE_LOG(LogTemp, Error, TEXT("设置Int属性值失败: 对象或属性为空"));
            return false;
        }
        
        FIntProperty* IntProperty = CastField<FIntProperty>(Property);
        if (!IntProperty)
        {
            UE_LOG(LogTemp, Error, TEXT("设置Int属性值失败: 属性类型不是Int"));
            return false;
        }
        
        // 如果值没有变化，直接返回
        if (CurrentValue == InValue)
        {
            return false;
        }
        
        // 安全地设置属性值
        IntProperty->SetPropertyValue_InContainer(TargetObject, InValue);
        CurrentValue = InValue;
        
        // 只更新当值不同时 - ValueSpinBox由BindWidget保证不为空
        if (FMath::RoundToInt(ValueSpinBox->GetValue()) != InValue)
        {
            ValueSpinBox->SetValue(InValue);
        }
        
        // 确保属性名称文本正确显示 - PropertyNameText由BindWidget保证不为空
        PropertyNameText->SetText(PropertyDisplayName);
        
        // 通知属性值已更改
        NotifyPropertyValueChanged();
        return true;
    }, TEXT("设置Int属性值失败"));
}

int32 URapidIntPropertyWidget::GetValue() const
{
    // 直接返回缓存值，无需重新从属性获取
    return CurrentValue;
}

void URapidIntPropertyWidget::HandleValueChanged(float NewValue)
{
    SafeExecute([&, NewValue]() {
        // 四舍五入为整数
        int32 NewIntValue = FMath::RoundToInt(NewValue);
        
        // 只有在值发生变化时才更新
        if (CurrentValue != NewIntValue)
        {
            SetValue(NewIntValue);
        }
    }, TEXT("处理数值变化失败"));
}