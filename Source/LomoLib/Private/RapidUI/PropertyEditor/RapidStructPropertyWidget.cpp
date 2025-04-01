// Fill out your copyright notice in the Description page of Project Settings.

#include "RapidUI/PropertyEditor/RapidStructPropertyWidget.h"
#include "Components/TextBlock.h"
#include "Components/ExpandableArea.h"
#include "Components/VerticalBox.h"
#include "UObject/UnrealType.h"
#include "RapidUI/PropertyEditor/RapidFloatPropertyWidget.h"
#include "RapidUI/PropertyEditor/RapidIntPropertyWidget.h"
#include "RapidUI/PropertyEditor/RapidBoolPropertyWidget.h"
#include "RapidUI/PropertyEditor/RapidStringPropertyWidget.h"
#include "Blueprint/WidgetTree.h"

URapidStructPropertyWidget::URapidStructPropertyWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

bool URapidStructPropertyWidget::InitializePropertyWidget(UObject* InObject, FProperty* InProperty, const FName& InPropertyName)
{
    // 调用父类的初始化方法
    if (!Super::InitializePropertyWidget(InObject, InProperty, InPropertyName))
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidStructPropertyWidget初始化失败: 父类初始化返回false"));
        return false;
    }

    // 参数校验
    if (!InObject || !InProperty)
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidStructPropertyWidget初始化失败: 对象或属性为空"));
        return false;
    }
    
    // 确保属性类型正确
    if (!InProperty->IsA<FStructProperty>())
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidStructPropertyWidget初始化失败: 属性类型不是Struct"));
        return false;
    }
    
    // 设置结构体标题
    if (TitleText)
    {
        TitleText->SetText(PropertyDisplayName);
    }
    
    // 清除现有的子属性控件
    ChildPropertyWidgets.Empty();
    if (ContentVerticalBox)
    {
        ContentVerticalBox->ClearChildren();
    }
    
    // 创建所有子属性控件
    CreateChildProperties();
    
    // 默认展开
    if (ExpandableArea)
    {
        // 可通过元数据控制是否默认展开
        bool bDefaultExpanded = true;
        if (Property && Property->HasMetaData(TEXT("DefaultExpanded")))
        {
            bDefaultExpanded = Property->GetMetaDataText(TEXT("DefaultExpanded")).ToString().ToBool();
        }
        ExpandableArea->SetIsExpanded(bDefaultExpanded);
    }
    
    return true;
}

void URapidStructPropertyWidget::UpdateValue_Implementation()
{
    // 更新所有子属性控件
    for (URapidPropertyWidget* ChildWidget : ChildPropertyWidgets)
    {
        if (ChildWidget)
        {
            ChildWidget->UpdateValue();
        }
    }
}

void URapidStructPropertyWidget::CreateChildProperties()
{
    SafeExecute([&]() {
        if (!Property || !TargetObject || !ContentVerticalBox)
        {
            UE_LOG(LogTemp, Warning, TEXT("CreateChildProperties失败: 属性、对象或内容框为空"));
            return;
        }
        
        // 获取结构体属性
        FStructProperty* StructProperty = CastField<FStructProperty>(Property);
        if (!StructProperty)
        {
            UE_LOG(LogTemp, Warning, TEXT("CreateChildProperties失败: 无法转换为结构体属性"));
            return;
        }
        
        // 获取结构体信息
        UScriptStruct* ScriptStruct = StructProperty->Struct;
        if (!ScriptStruct)
        {
            UE_LOG(LogTemp, Warning, TEXT("CreateChildProperties失败: 结构体描述为空"));
            return;
        }
        
        // 获取结构体值的指针
        void* StructValuePtr = StructProperty->ContainerPtrToValuePtr<void>(TargetObject);
        if (!StructValuePtr)
        {
            UE_LOG(LogTemp, Warning, TEXT("CreateChildProperties失败: 结构体值指针为空"));
            return;
        }
        
        // 获取结构体中的所有属性
        for (TFieldIterator<FProperty> It(ScriptStruct); It; ++It)
        {
            FProperty* StructField = *It;
            
            // 跳过不可编辑的属性
            if (!StructField->HasAnyPropertyFlags(CPF_Edit))
            {
                continue;
            }
            
            // 跳过仅编辑器可见的属性
            if (StructField->HasAnyPropertyFlags(CPF_EditorOnly))
            {
                continue;
            }
            
            // 获取字段地址
            void* FieldValuePtr = StructField->ContainerPtrToValuePtr<void>(StructValuePtr);
            
            // 创建字段控件
            URapidPropertyWidget* FieldWidget = nullptr;
            
            // 根据字段类型创建不同的控件
            if (StructField->IsA<FFloatProperty>())
            {
                FieldWidget = CreateWidget<URapidFloatPropertyWidget>(this, URapidFloatPropertyWidget::StaticClass());
            }
            else if (StructField->IsA<FIntProperty>())
            {
                FieldWidget = CreateWidget<URapidIntPropertyWidget>(this, URapidIntPropertyWidget::StaticClass());
            }
            else if (StructField->IsA<FBoolProperty>())
            {
                FieldWidget = CreateWidget<URapidBoolPropertyWidget>(this, URapidBoolPropertyWidget::StaticClass());
            }
            else if (StructField->IsA<FStrProperty>() || StructField->IsA<FNameProperty>() || StructField->IsA<FTextProperty>())
            {
                FieldWidget = CreateWidget<URapidStringPropertyWidget>(this, URapidStringPropertyWidget::StaticClass());
            }
            else if (StructField->IsA<FStructProperty>())
            {
                FieldWidget = CreateWidget<URapidStructPropertyWidget>(this, URapidStructPropertyWidget::StaticClass());
            }
            
            // 初始化字段控件
            if (FieldWidget)
            {
                // 设置特殊属性名格式以便识别结构体字段
                FName FieldPropertyName = *FString::Printf(TEXT("%s.%s"), *PropertyName.ToString(), *StructField->GetName());
                
                // 初始化控件
                if (FieldWidget->InitializePropertyWidget(TargetObject, StructField, FieldPropertyName))
                {
                    // 绑定值变化事件
                    FieldWidget->OnPropertyValueChanged.AddDynamic(this, &URapidStructPropertyWidget::HandleChildPropertyValueChanged);
                    
                    // 添加到容器
                    ContentVerticalBox->AddChild(FieldWidget);
                    ChildPropertyWidgets.Add(FieldWidget);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("无法初始化字段控件: %s"), *FieldPropertyName.ToString());
                }
            }
        }
    }, TEXT("创建结构体子属性控件失败"));
}

void URapidStructPropertyWidget::HandleChildPropertyValueChanged(UObject* Object, FName InPropertyName, URapidPropertyWidget* PropertyWidget)
{
    // 传递子属性变化事件
    NotifyPropertyValueChanged();
}