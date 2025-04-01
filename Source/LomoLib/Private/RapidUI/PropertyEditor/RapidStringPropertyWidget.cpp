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

    // 输出调试信息
    UE_LOG(LogTemp, Display, TEXT("正在初始化StringPropertyWidget - 对象: %s, 属性: %s"), 
           *InObject->GetName(), *InProperty->GetName());

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

    // 设置属性名称文本 - PropertyNameText由BindWidget保证不为空
    PropertyNameText->SetText(PropertyDisplayName);
    UE_LOG(LogTemp, Display, TEXT("设置属性名称文本: %s"), *PropertyDisplayName.ToString());

    // 绑定文本框事件 - ValueTextBox由BindWidget保证不为空
    ValueTextBox->OnTextCommitted.AddDynamic(this, &URapidStringPropertyWidget::HandleTextCommitted);
    ValueTextBox->OnTextChanged.AddDynamic(this, &URapidStringPropertyWidget::HandleTextChanged);
    UE_LOG(LogTemp, Display, TEXT("文本框事件绑定成功"));

    // 从目标对象读取初始值
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
                UE_LOG(LogTemp, Display, TEXT("获取FString属性值: %s"), *CurrentValue);
            }
        }
        else if (PropertyType == EPropertyType::Name)
        {
            FNameProperty* NameProperty = CastField<FNameProperty>(Property);
            if (NameProperty && TargetObject)
            {
                FName NameValue = NameProperty->GetPropertyValue_InContainer(TargetObject);
                CurrentValue = NameValue.ToString();
                UE_LOG(LogTemp, Display, TEXT("获取FName属性值: %s"), *CurrentValue);
            }
        }
        else if (PropertyType == EPropertyType::Text)
        {
            FTextProperty* TextProperty = CastField<FTextProperty>(Property);
            if (TextProperty && TargetObject)
            {
                FText TextValue = TextProperty->GetPropertyValue_InContainer(TargetObject);
                CurrentValue = TextValue.ToString();
                UE_LOG(LogTemp, Display, TEXT("获取FText属性值: %s"), *CurrentValue);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("无法更新值: 未知的属性类型"));
            return;
        }

        // 更新界面 - ValueTextBox由BindWidget保证不为空
        ValueTextBox->SetText(FText::FromString(CurrentValue));
        UE_LOG(LogTemp, Display, TEXT("界面文本框已更新为: %s"), *CurrentValue);
        
        // 确保属性名称文本正确显示 - PropertyNameText由BindWidget保证不为空
        PropertyNameText->SetText(PropertyDisplayName);
        UE_LOG(LogTemp, Display, TEXT("属性名称文本已更新为: %s"), *PropertyDisplayName.ToString());
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

        UE_LOG(LogTemp, Display, TEXT("尝试设置String属性值: %s"), *InValue);

        // 如果值没有变化，直接返回
        if (CurrentValue.Equals(InValue))
        {
            UE_LOG(LogTemp, Display, TEXT("值未变化，跳过设置"));
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
                UE_LOG(LogTemp, Display, TEXT("FString属性值已设置"));
            }
        }
        else if (PropertyType == EPropertyType::Name)
        {
            FNameProperty* NameProperty = CastField<FNameProperty>(Property);
            if (NameProperty)
            {
                NameProperty->SetPropertyValue_InContainer(TargetObject, FName(*InValue));
                bValueSet = true;
                UE_LOG(LogTemp, Display, TEXT("FName属性值已设置"));
            }
        }
        else if (PropertyType == EPropertyType::Text)
        {
            FTextProperty* TextProperty = CastField<FTextProperty>(Property);
            if (TextProperty)
            {
                TextProperty->SetPropertyValue_InContainer(TargetObject, FText::FromString(InValue));
                bValueSet = true;
                UE_LOG(LogTemp, Display, TEXT("FText属性值已设置"));
            }
        }

        if (bValueSet)
        {
            // 更新缓存的当前值
            CurrentValue = InValue;
            
            // 更新UI - ValueTextBox由BindWidget保证不为空
            ValueTextBox->SetText(FText::FromString(CurrentValue));

            // 确保属性名称文本正确显示 - PropertyNameText由BindWidget保证不为空
            PropertyNameText->SetText(PropertyDisplayName);
            UE_LOG(LogTemp, Display, TEXT("属性名称文本已更新为: %s"), *PropertyDisplayName.ToString());

            // 通知值变化
            NotifyPropertyValueChanged();
            UE_LOG(LogTemp, Display, TEXT("属性值已变更并通知: %s"), *CurrentValue);
            return true;
        }

        UE_LOG(LogTemp, Error, TEXT("设置属性值失败"));
        return false;
    },
    TEXT("设置String属性值失败"));
}

FString URapidStringPropertyWidget::GetValue() const
{
    // 直接返回缓存的值，避免每次都从属性获取
    return CurrentValue;
}

void URapidStringPropertyWidget::HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    SafeExecute([&]()
    {
        // 只在按下回车键或失去焦点时更新值
        if (CommitMethod == ETextCommit::OnEnter || CommitMethod == ETextCommit::OnUserMovedFocus)
        {
            FString NewValue = Text.ToString();
            UE_LOG(LogTemp, Display, TEXT("文本已提交，新值: %s, 提交方式: %d"), *NewValue, (int32)CommitMethod);
            SetValue(NewValue);
        }
    },
    TEXT("处理文本提交失败"));
}

void URapidStringPropertyWidget::HandleTextChanged(const FText& Text)
{
    // 这里可以添加实时验证或格式化，但暂时不会立即更新值
    UE_LOG(LogTemp, Display, TEXT("文本正在变更: %s"), *Text.ToString());
}