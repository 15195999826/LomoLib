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
    if (!Property || !ValuePtr)
    {
        return;
    }
    
    FText DisplayText;
    
    // 根据属性类型获取值
    if (PropertyType == EPropertyType::String)
    {
        FStrProperty* StrProperty = CastField<FStrProperty>(Property);
        if (StrProperty)
        {
            CurrentValue = StrProperty->GetPropertyValue(ValuePtr);
            DisplayText = FText::FromString(CurrentValue);
        }
    }
    else if (PropertyType == EPropertyType::Name)
    {
        FNameProperty* NameProperty = CastField<FNameProperty>(Property);
        if (NameProperty)
        {
            FName NameValue = NameProperty->GetPropertyValue(ValuePtr);
            CurrentValue = NameValue.ToString();
            DisplayText = FText::FromName(NameValue);
        }
    }
    else if (PropertyType == EPropertyType::Text)
    {
        FTextProperty* TextProperty = CastField<FTextProperty>(Property);
        if (TextProperty)
        {
            FText TextValue = TextProperty->GetPropertyValue(ValuePtr);
            CurrentValue = TextValue.ToString();
            DisplayText = TextValue;
        }
    }
    
    // 更新UI
    if (ValueTextBox)
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
    
    // 根据属性类型设置值
    if (PropertyType == EPropertyType::String)
    {
        FStrProperty* StrProperty = CastField<FStrProperty>(Property);
        if (StrProperty)
        {
            StrProperty->SetPropertyValue(ValuePtr, InValue);
        }
    }
    else if (PropertyType == EPropertyType::Name)
    {
        FNameProperty* NameProperty = CastField<FNameProperty>(Property);
        if (NameProperty)
        {
            FName NameValue = FName(*InValue);
            NameProperty->SetPropertyValue(ValuePtr, NameValue);
        }
    }
    else if (PropertyType == EPropertyType::Text)
    {
        FTextProperty* TextProperty = CastField<FTextProperty>(Property);
        if (TextProperty)
        {
            FText TextValue = FText::FromString(InValue);
            TextProperty->SetPropertyValue(ValuePtr, TextValue);
        }
    }
    
    NotifyPropertyValueChanged();
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