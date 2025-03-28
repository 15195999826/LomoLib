// Fill out your copyright notice in the Description page of Project Settings.

#include "RapidUI/PropertyEditor/RapidPropertyEditor.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "UObject/UnrealType.h"
#include "UObject/PropertyPortFlags.h"
#include "RapidUI/PropertyEditor/RapidPropertyWidget.h"

void URapidPropertyEditor::NativeConstruct()
{
    Super::NativeConstruct();
}

void URapidPropertyEditor::NativeDestruct()
{
    Super::NativeDestruct();
    ClearObject();
}

bool URapidPropertyEditor::SetObject(UObject* InObject)
{
    // 清空现有的内容
    if (ContentScrollBox)
    {
        ContentScrollBox->ClearChildren();
    }
    
    // 清空已创建的属性控件
    PropertyWidgets.Empty();
    
    // 如果传入的对象为空，则清除当前对象并返回
    if (!InObject)
    {
        TargetObject = nullptr;
        return false;
    }
    
    // 设置新对象并渲染属性
    TargetObject = InObject;
    RenderProperties();
    return true;
}

UObject* URapidPropertyEditor::GetObject() const
{
    return TargetObject;
}

void URapidPropertyEditor::Refresh()
{
    if (TargetObject)
    {
        // 清空已创建的属性控件
        PropertyWidgets.Empty();
        
        RenderProperties();
    }
}

void URapidPropertyEditor::ClearObject()
{
    if (ContentScrollBox)
    {
        ContentScrollBox->ClearChildren();
    }
    
    // 清空已创建的属性控件
    PropertyWidgets.Empty();
    
    TargetObject = nullptr;
}

TObjectPtr<URapidPropertyWidget> URapidPropertyEditor::CreatePropertyWidgetForType(UUserWidget* InOuter, const FProperty* InProperty)
{
    if (!InProperty)
    {
        return nullptr;
    }

    // 定义属性类型检查辅助函数
    auto IsStringProperty = [](const FProperty* Prop) -> bool {
        return Prop->IsA<FStrProperty>() || Prop->IsA<FNameProperty>() || Prop->IsA<FTextProperty>();
    };

    auto IsEnumProperty = [](const FProperty* Prop) -> bool {
        if (Prop->IsA<FEnumProperty>())
            return true;
        if (const FByteProperty* ByteProp = CastField<FByteProperty>(Prop))
            return ByteProp->IsEnum();
        return false;
    };

    TSubclassOf<URapidPropertyWidget> WidgetClass = nullptr;

    if (InProperty->IsA<FFloatProperty>())
    {
        WidgetClass = FloatPropertyWidgetClass;
    }
    else if (InProperty->IsA<FIntProperty>())
    {
        WidgetClass = IntPropertyWidgetClass;
    }
    else if (InProperty->IsA<FBoolProperty>())
    {
        WidgetClass = BoolPropertyWidgetClass;
    }
    else if (IsStringProperty(InProperty))
    {
        WidgetClass = StringPropertyWidgetClass;
    }
    else if (IsEnumProperty(InProperty))
    {
        WidgetClass = StringPropertyWidgetClass;
    }
    else if (InProperty->IsA<FStructProperty>())
    {
        WidgetClass = StructPropertyWidgetClass;
    }
    else if (InProperty->IsA<FArrayProperty>())
    {
        WidgetClass = ArrayPropertyWidgetClass;
    }
    else if (InProperty->IsA<FMapProperty>())
    {
        WidgetClass = MapPropertyWidgetClass;
    }

    check(WidgetClass);

    return CreateWidget<URapidPropertyWidget>(InOuter, WidgetClass);
}

void URapidPropertyEditor::RenderProperties()
{
    if (!ContentScrollBox || !TargetObject)
    {
        return;
    }
    
    // 清空现有内容
    ContentScrollBox->ClearChildren();
    
    // 获取对象的类
    UClass* ObjectClass = TargetObject->GetClass();
    
    // 创建垂直容器
    UVerticalBox* MainVerticalBox = NewObject<UVerticalBox>(this);
    ContentScrollBox->AddChild(MainVerticalBox);
    
    // 遍历所有属性
    for (TFieldIterator<FProperty> PropIt(ObjectClass); PropIt; ++PropIt)
    {
        FProperty* Property = *PropIt;
        
        // 跳过不可编辑的属性
        if (!Property->HasAnyPropertyFlags(CPF_Edit))
        {
            continue;
        }
            
        // 跳过仅编辑器可见的属性
        if (Property->HasAnyPropertyFlags(CPF_EditorOnly))
        {
            continue;
        }
        
        // 获取属性地址
        void* PropertyValuePtr = Property->ContainerPtrToValuePtr<void>(TargetObject);
        
        // 创建属性控件
        auto PropertyWidget = CreatePropertyWidgetForType(this, Property);
        
        // 初始化属性控件
        PropertyWidget->InitializePropertyWidget(TargetObject, Property, PropertyValuePtr);
            
        // 绑定属性值变化事件
        PropertyWidget->OnPropertyValueChanged.AddDynamic(this, &URapidPropertyEditor::HandlePropertyValueChanged);
            
        // 添加到主容器
        MainVerticalBox->AddChild(PropertyWidget);
            
        // 添加到控件列表
        PropertyWidgets.Add(PropertyWidget);
    }
}

void URapidPropertyEditor::HandlePropertyValueChanged(UObject* Object, FName PropertyName, URapidPropertyWidget* PropertyWidget)
{
    // 触发属性改变事件
    if (Object == TargetObject)
    {
        FProperty* Property = TargetObject->GetClass()->FindPropertyByName(PropertyName);
        if (Property)
        {
            NotifyPropertyChanged(PropertyName, Property);
        }
    }
}

FString URapidPropertyEditor::GetPropertyDisplayName(FProperty* Property) const
{
    if (!Property)
    {
        return TEXT("");
    }
    
    // 使用DisplayName元数据如果存在
    if (Property->HasMetaData(TEXT("DisplayName")))
    {
        return Property->GetMetaData(TEXT("DisplayName"));
    }
    
    // 否则使用属性名
    return Property->GetName();
}

void URapidPropertyEditor::NotifyPropertyChanged(FName PropertyName, FProperty* Property)
{
    // 广播属性改变事件
    // OnPropertyChanged.Broadcast(TargetObject, PropertyName, Property);
}