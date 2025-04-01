// Fill out your copyright notice in the Description page of Project Settings.

#include "RapidUI/PropertyEditor/RapidStringPropertyWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "UObject/UnrealType.h"

URapidStringPropertyWidget::URapidStringPropertyWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , CurrentValue("")
    , PropertyType(EPropertyType::String)
{
}

void URapidStringPropertyWidget::InitializePropertyWidget(UObject* InObject, FProperty* InProperty, void* InValuePtr)
{
    Super::InitializePropertyWidget(InObject, InProperty, InValuePtr);
    
    // 确定属性类型
    if (Property->IsA<FStrProperty>())
    {
        PropertyType = EPropertyType::String;
    }
    else if (Property->IsA<FNameProperty>())
    {
        PropertyType = EPropertyType::Name;
    }
    else if (Property->IsA<FTextProperty>())
    {
        PropertyType = EPropertyType::Text;
    }
    
    // 绑定文本框事件
    if (ValueTextBox)
    {
        ValueTextBox->OnTextCommitted.AddDynamic(this, &URapidStringPropertyWidget::HandleTextCommitted);
        ValueTextBox->OnTextChanged.AddDynamic(this, &URapidStringPropertyWidget::HandleTextChanged);
    }
    
    // 设置属性名称
    if (PropertyNameText)
    {
        PropertyNameText->SetText(PropertyDisplayName);
    }
    
    // 更新初始值
    UpdateValue();
}

void URapidStringPropertyWidget::UpdateValue_Implementation()
{
    if (!Property || !ValuePtr || !TargetObject)
    {
        return;
    }
    
    // 初始化为空值
    CurrentValue = TEXT("");
    FText DisplayText = FText::GetEmpty();
    
    // 根据属性类型使用基础方法获取值
    if (Property && Property->IsValidLowLevel())
    {
        // if (const FStrProperty* StrProperty = CastField<FStrProperty>(Property))
        // {
        //     const FString* Value = StrProperty->GetPropertyValuePtr(ValuePtr);
        //     if (Value)
        //     {
        //         CurrentValue = *Value;
        //         DisplayText = FText::FromString(CurrentValue);
        //     }
        // }
        // else if (const FNameProperty* NameProperty = CastField<FNameProperty>(Property))
        // {
        //     const FName* Value = NameProperty->GetPropertyValuePtr(ValuePtr);
        //     if (Value)
        //     {
        //         CurrentValue = Value->ToString();
        //         DisplayText = FText::FromName(*Value);
        //     }
        // }
        // else if (const FTextProperty* TextProperty = CastField<FTextProperty>(Property))
        // {
        //     const FText* Value = TextProperty->GetPropertyValuePtr(ValuePtr);
        //     if (Value)
        //     {
        //         CurrentValue = Value->ToString();
        //         DisplayText = *Value;
        //     }
        // }
    }
    
    // 更新UI
    if (ValueTextBox && ValueTextBox->IsValidLowLevel())
    {
        ValueTextBox->SetText(DisplayText);
    }
}

void URapidStringPropertyWidget::SetValue(const FString& InValue)
{
    if (!Property || !ValuePtr || !TargetObject)
    {
        return;
    }
    
    CurrentValue = InValue;
    bool bValueChanged = false;
    
    // 根据属性类型使用基础方法设置值
    if (Property && Property->IsValidLowLevel())
    {
        if (FStrProperty* StrProperty = CastField<FStrProperty>(Property))
        {
            FString* Value = StrProperty->GetPropertyValuePtr(ValuePtr);
            if (Value)
            {
                *Value = InValue;
                bValueChanged = true;
            }
        }
        else if (FNameProperty* NameProperty = CastField<FNameProperty>(Property))
        {
            FName* Value = NameProperty->GetPropertyValuePtr(ValuePtr);
            if (Value)
            {
                *Value = FName(*InValue);
                bValueChanged = true;
            }
        }
        else if (FTextProperty* TextProperty = CastField<FTextProperty>(Property))
        {
            FText* Value = TextProperty->GetPropertyValuePtr(ValuePtr);
            if (Value)
            {
                *Value = FText::FromString(InValue);
                bValueChanged = true;
            }
        }
    }
    
    if (bValueChanged)
    {
        NotifyPropertyValueChanged();
    }
}

FString URapidStringPropertyWidget::GetValue() const
{
    return CurrentValue;
}

void URapidStringPropertyWidget::HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod == ETextCommit::OnEnter || CommitMethod == ETextCommit::OnUserMovedFocus)
    {
        FString NewValue = Text.ToString();
        if (CurrentValue != NewValue)
        {
            SetValue(NewValue);
        }
    }
}

void URapidStringPropertyWidget::HandleTextChanged(const FText& Text)
{
    // 可以在此处添加实时验证逻辑，但不要立即更新属性值
}