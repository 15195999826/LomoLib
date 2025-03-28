 // Fill out your copyright notice in the Description page of Project Settings.

#include "RapidUI/PropertyEditor/RapidPropertyWidget.h"
#include "UObject/UnrealType.h"

URapidPropertyWidget::URapidPropertyWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , TargetObject(nullptr)
    , Property(nullptr)
    , ValuePtr(nullptr)
{
}

FName URapidPropertyWidget::GetPropertyName() const
{
    return PropertyName;
}

void URapidPropertyWidget::SetPropertyName(FName InPropertyName)
{
    PropertyName = InPropertyName;
}

FText URapidPropertyWidget::GetPropertyDisplayName() const
{
    return PropertyDisplayName;
}

void URapidPropertyWidget::SetPropertyDisplayName(const FText& InDisplayName)
{
    PropertyDisplayName = InDisplayName;
}

void URapidPropertyWidget::InitializePropertyWidget(UObject* InObject, FProperty* InProperty, void* InValuePtr)
{
    TargetObject = InObject;
    Property = InProperty;
    ValuePtr = InValuePtr;
    
    if (Property)
    {
        PropertyName = Property->GetFName();
        
        // 设置显示名称
        if (Property->HasMetaData(TEXT("DisplayName")))
        {
            PropertyDisplayName = FText::FromString(Property->GetMetaData(TEXT("DisplayName")));
        }
        else
        {
            PropertyDisplayName = FText::FromName(PropertyName);
        }
        
        UpdateValue();
    }
}

void URapidPropertyWidget::UpdateValue_Implementation()
{
    // 子类中实现实际更新逻辑
}

FString URapidPropertyWidget::GetPropertyMetaData(const FName& MetaDataKey) const
{
    if (Property && Property->HasMetaData(MetaDataKey))
    {
        return Property->GetMetaData(MetaDataKey);
    }
    
    return FString();
}

void URapidPropertyWidget::NotifyPropertyValueChanged()
{
    if (TargetObject && !PropertyName.IsNone())
    {
        OnPropertyValueChanged.Broadcast(TargetObject, PropertyName, this);
    }
}