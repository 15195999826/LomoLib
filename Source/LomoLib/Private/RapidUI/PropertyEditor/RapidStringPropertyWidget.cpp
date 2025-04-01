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

bool URapidStringPropertyWidget::InitializePropertyWidget(UObject* InObject, FProperty* InProperty, const FName& InPropertyName)
{
    // 调用父类的初始化方法
    if (!Super::InitializePropertyWidget(InObject, InProperty, InPropertyName))
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidStringPropertyWidget初始化失败: 父类初始化返回false"));
        return false;
    }

    // 参数校验
    if (!InObject || !InProperty)
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidStringPropertyWidget初始化失败: 对象或属性为空"));
        return false;
    }

    // 确定属性类型
    if (InProperty->IsA<FStrProperty>())
    {
        PropertyType = EPropertyType::String;
        UE_LOG(LogTemp, Display, TEXT("属性类型: FString"));
    }
    else if (InProperty->IsA<FNameProperty>())
    {
        PropertyType = EPropertyType::Name;
        UE_LOG(LogTemp, Display, TEXT("属性类型: FName"));
    }
    else if (InProperty->IsA<FTextProperty>())
    {
        PropertyType = EPropertyType::Text;
        UE_LOG(LogTemp, Display, TEXT("属性类型: FText"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("RapidStringPropertyWidget初始化失败: 不支持的属性类型 %s"), 
               *InProperty->GetClass()->GetName());
        return false;
    }

    // 设置属性名称文本
    PropertyNameText->SetText(PropertyDisplayName);
    
    // 绑定文本框事件
    ValueTextBox->OnTextCommitted.AddDynamic(this, &URapidStringPropertyWidget::HandleTextCommitted);
    ValueTextBox->OnTextChanged.AddDynamic(this, &URapidStringPropertyWidget::HandleTextChanged);

    // 更新初始值
    UpdateValue();
    return true;
}

void URapidStringPropertyWidget::UpdateValue_Implementation()
{
    SafeExecute([&]()
    {
        // 初始化当前值
        CurrentValue = FString();

        // 获取属性值
        if (PropertyType == EPropertyType::String)
        {
            FStrProperty* StrProperty = CastField<FStrProperty>(Property);
            if (StrProperty && TargetObject)
            {
                CurrentValue = StrProperty->GetPropertyValue_InContainer(TargetObject);
                UE_LOG(LogTemp, Verbose, TEXT("获取FString属性值: %s"), *CurrentValue);
            }
        }
        else if (PropertyType == EPropertyType::Name)
        {
            FNameProperty* NameProperty = CastField<FNameProperty>(Property);
            if (NameProperty && TargetObject)
            {
                FName NameValue = NameProperty->GetPropertyValue_InContainer(TargetObject);
                CurrentValue = NameValue.ToString();
                UE_LOG(LogTemp, Verbose, TEXT("获取FName属性值: %s"), *CurrentValue);
            }
        }
        else if (PropertyType == EPropertyType::Text)
        {
            FTextProperty* TextProperty = CastField<FTextProperty>(Property);
            if (TextProperty && TargetObject)
            {
                FText TextValue = TextProperty->GetPropertyValue_InContainer(TargetObject);
                CurrentValue = TextValue.ToString();
                UE_LOG(LogTemp, Verbose, TEXT("获取FText属性值: %s"), *CurrentValue);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("无法更新值: 未知的属性类型"));
            return;
        }

        // 更新界面
        ValueTextBox->SetText(FText::FromString(CurrentValue));
        PropertyNameText->SetText(PropertyDisplayName);
    },
    TEXT("更新String属性值失败"));
}

bool URapidStringPropertyWidget::SetValue(const FString& InValue)
{
    return SafeExecuteWithRet([&]() -> bool
    {
        if (!TargetObject || !Property)
        {
            UE_LOG(LogTemp, Error, TEXT("设置String属性值失败: 对象或属性为空"));
            return false;
        }

        // 如果值没有变化，直接返回
        if (CurrentValue.Equals(InValue))
        {
            return false;
        }

        bool bValueSet = false;

        // 根据属性类型设置值
        if (PropertyType == EPropertyType::String)
        {
            FStrProperty* StrProperty = CastField<FStrProperty>(Property);
            if (StrProperty)
            {
                StrProperty->SetPropertyValue_InContainer(TargetObject, InValue);
                bValueSet = true;
            }
        }
        else if (PropertyType == EPropertyType::Name)
        {
            FNameProperty* NameProperty = CastField<FNameProperty>(Property);
            if (NameProperty)
            {
                NameProperty->SetPropertyValue_InContainer(TargetObject, FName(*InValue));
                bValueSet = true;
            }
        }
        else if (PropertyType == EPropertyType::Text)
        {
            FTextProperty* TextProperty = CastField<FTextProperty>(Property);
            if (TextProperty)
            {
                TextProperty->SetPropertyValue_InContainer(TargetObject, FText::FromString(InValue));
                bValueSet = true;
            }
        }

        if (bValueSet)
        {
            // 更新缓存的当前值
            CurrentValue = InValue;
            
            // 更新UI
            ValueTextBox->SetText(FText::FromString(CurrentValue));
            PropertyNameText->SetText(PropertyDisplayName);

            // 通知值变化
            NotifyPropertyValueChanged();
            return true;
        }

        UE_LOG(LogTemp, Error, TEXT("设置属性值失败"));
        return false;
    },
    TEXT("设置String属性值失败"));
}

FString URapidStringPropertyWidget::GetValue() const
{
    return CurrentValue;
}

void URapidStringPropertyWidget::HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    SafeExecute([&]()
    {
        // 只在按下回车键或失去焦点时更新值
        if (CommitMethod == ETextCommit::OnEnter || CommitMethod == ETextCommit::OnUserMovedFocus)
        {
            SetValue(Text.ToString());
        }
    },
    TEXT("处理文本提交失败"));
}

void URapidStringPropertyWidget::HandleTextChanged(const FText& Text)
{
    // 这里可以添加实时验证或格式化，但暂时不会立即更新值
    UE_LOG(LogTemp, Verbose, TEXT("文本正在变更: %s"), *Text.ToString());
}