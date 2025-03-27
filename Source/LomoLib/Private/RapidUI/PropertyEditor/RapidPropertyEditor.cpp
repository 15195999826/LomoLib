// Fill out your copyright notice in the Description page of Project Settings.

#include "RapidUI/PropertyEditor/RapidPropertyEditor.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/SpinBox.h"
#include "Components/CheckBox.h"
#include "Components/ExpandableArea.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "UObject/UnrealType.h"
#include "UObject/PropertyPortFlags.h"
#include "Internationalization/Text.h"

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
    
    // 清空控件到属性的映射
    WidgetToPropertyMap.Empty();
    
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
        // 清空控件到属性的映射
        WidgetToPropertyMap.Empty();
        
        RenderProperties();
    }
}

void URapidPropertyEditor::ClearObject()
{
    if (ContentScrollBox)
    {
        ContentScrollBox->ClearChildren();
    }
    
    // 清空控件到属性的映射
    WidgetToPropertyMap.Empty();
    
    TargetObject = nullptr;
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
        UWidget* PropertyWidget = CreatePropertyWidget(Property, PropertyValuePtr);
        
        if (PropertyWidget)
        {
            // 设置控件对应的属性名称
            SetWidgetPropertyName(PropertyWidget, Property->GetFName());
            
            // 创建属性行并添加到主容器
            FString PropertyDisplayName = GetPropertyDisplayName(Property);
            UWidget* PropertyRow = CreatePropertyRow(PropertyDisplayName, PropertyWidget, 
                (Property->IsA<FStructProperty>() || Property->IsA<FArrayProperty>() || Property->IsA<FMapProperty>()));
            MainVerticalBox->AddChild(PropertyRow);
        }
    }
}

UWidget* URapidPropertyEditor::CreatePropertyWidget(FProperty* Property, void* PropertyValue)
{
    if (!Property || !PropertyValue)
    {
        return nullptr;
    }
    
    // 根据属性类型创建不同的控件
    if (Property->IsA<FStructProperty>())
    {
        return CreateStructPropertyWidget(CastField<FStructProperty>(Property), PropertyValue);
    }

    if (Property->IsA<FArrayProperty>())
    {
        return CreateArrayPropertyWidget(CastField<FArrayProperty>(Property), PropertyValue);
    }

    if (Property->IsA<FMapProperty>())
    {
        return CreateMapPropertyWidget(CastField<FMapProperty>(Property), PropertyValue);
    }

    return CreateBasicPropertyWidget(Property, PropertyValue);
}

UWidget* URapidPropertyEditor::CreateBasicPropertyWidget(FProperty* Property, void* PropertyValue)
{
    if (Property->IsA<FFloatProperty>())
    {
        // 创建Float编辑控件
        USpinBox* SpinBox = NewObject<USpinBox>(this);
        FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property);
        float Value = FloatProperty->GetPropertyValue(PropertyValue);
        
        SpinBox->SetValue(Value);
        SpinBox->SetMinValue(TNumericLimits<float>::Lowest());
        SpinBox->SetMaxValue(TNumericLimits<float>::Max());
        SpinBox->SetDelta(0.1f);
        
        // 绑定值变化事件
        SpinBox->OnValueChanged.AddDynamic(this, &URapidPropertyEditor::HandleFloatValueChanged);
        
        return SpinBox;
    }

    if (Property->IsA<FIntProperty>())
    {
        // 创建Int编辑控件
        USpinBox* SpinBox = NewObject<USpinBox>(this);
        FIntProperty* IntProperty = CastField<FIntProperty>(Property);
        int32 Value = IntProperty->GetPropertyValue(PropertyValue);
        
        SpinBox->SetValue(Value);
        SpinBox->SetMinValue(TNumericLimits<int32>::Lowest());
        SpinBox->SetMaxValue(TNumericLimits<int32>::Max());
        SpinBox->SetDelta(1.0f);
        SpinBox->SetMinSliderValue(TNumericLimits<int32>::Lowest());
        SpinBox->SetMaxSliderValue(TNumericLimits<int32>::Max());
        
        // 绑定值变化事件
        SpinBox->OnValueChanged.AddDynamic(this, &URapidPropertyEditor::HandleFloatValueChanged);
        
        return SpinBox;
    }
    if (Property->IsA<FBoolProperty>())
    {
        // 创建Bool编辑控件
        UCheckBox* CheckBox = NewObject<UCheckBox>(this);
        FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property);
        bool Value = BoolProperty->GetPropertyValue(PropertyValue);
        
        CheckBox->SetIsChecked(Value);
        
        // 绑定值变化事件
        CheckBox->OnCheckStateChanged.AddDynamic(this, &URapidPropertyEditor::HandleBoolValueChanged);
        
        return CheckBox;
    }
    if (Property->IsA<FStrProperty>())
    {
        // 创建String编辑控件
        UEditableTextBox* TextBox = NewObject<UEditableTextBox>(this);
        FStrProperty* StrProperty = CastField<FStrProperty>(Property);
        FString Value = StrProperty->GetPropertyValue(PropertyValue);
        
        TextBox->SetText(FText::FromString(Value));
        
        // 绑定值变化事件
        TextBox->OnTextCommitted.AddDynamic(this, &URapidPropertyEditor::HandleStringValueChanged);
        
        return TextBox;
    }

    if (Property->IsA<FNameProperty>())
    {
        // 创建Name编辑控件
        UEditableTextBox* TextBox = NewObject<UEditableTextBox>(this);
        FNameProperty* NameProperty = CastField<FNameProperty>(Property);
        FName Value = NameProperty->GetPropertyValue(PropertyValue);
        
        TextBox->SetText(FText::FromName(Value));
        
        // 绑定值变化事件
        TextBox->OnTextCommitted.AddDynamic(this, &URapidPropertyEditor::HandleStringValueChanged);
        
        return TextBox;
    }

    if (Property->IsA<FTextProperty>())
    {
        // 创建Text编辑控件
        UEditableTextBox* TextBox = NewObject<UEditableTextBox>(this);
        FTextProperty* TextProperty = CastField<FTextProperty>(Property);
        FText Value = TextProperty->GetPropertyValue(PropertyValue);
        
        TextBox->SetText(Value);
        
        // 绑定值变化事件
        TextBox->OnTextCommitted.AddDynamic(this, &URapidPropertyEditor::HandleStringValueChanged);
        
        return TextBox;
    }

    // 对于其他不支持的类型，显示文本表示
    UTextBlock* TextBlock = NewObject<UTextBlock>(this);
    FString ValueStr;
    TextBlock->SetText(FText::FromString(ValueStr));
    return TextBlock;
}

UWidget* URapidPropertyEditor::CreateArrayPropertyWidget(FArrayProperty* ArrayProperty, void* PropertyValue)
{
    if (!ArrayProperty || !PropertyValue)
    {
        return nullptr;
    }
    
    // 创建一个可展开区域
    UExpandableArea* ExpandableArea = NewObject<UExpandableArea>(this);
    ExpandableArea->SetIsExpanded(false);
    
    // 创建一个垂直容器放入数组元素
    UVerticalBox* ArrayVerticalBox = NewObject<UVerticalBox>(this);
    
    // 获取数组
    FScriptArrayHelper ArrayHelper(ArrayProperty, PropertyValue);
    int32 ElementCount = ArrayHelper.Num();
    
    // 创建数组标题
    UTextBlock* HeaderText = NewObject<UTextBlock>(this);
    HeaderText->SetText(FText::Format(FText::FromString("{0} [{1}]"), 
        FText::FromString(ArrayProperty->GetName()), 
        FText::AsNumber(ElementCount)));
    
    // ExpandableArea->SetHeaderContent(HeaderText);
    ExpandableArea->SetContentForSlot(TEXT("Body"), ArrayVerticalBox);
    
    // 获取数组元素类型
    FProperty* InnerProperty = ArrayProperty->Inner;
    
    // 添加数组元素
    for (int32 i = 0; i < ElementCount; ++i)
    {
        UHorizontalBox* ElementRow = NewObject<UHorizontalBox>(this);
        
        // 创建元素索引标签
        UTextBlock* IndexText = NewObject<UTextBlock>(this);
        IndexText->SetText(FText::Format(FText::FromString("[{0}]:"), FText::AsNumber(i)));
        ElementRow->AddChild(IndexText);
        
        // 创建元素值控件
        void* ElementValuePtr = ArrayHelper.GetRawPtr(i);
        UWidget* ElementWidget = CreatePropertyWidget(InnerProperty, ElementValuePtr);
        
        if (ElementWidget)
        {
            // 设置数组元素控件的属性名称（使用特殊格式以便识别数组元素）
            FName ElementPropertyName = *FString::Printf(TEXT("%s[%d]"), *ArrayProperty->GetName(), i);
            SetWidgetPropertyName(ElementWidget, ElementPropertyName);
            
            ElementRow->AddChild(ElementWidget);
        }
        
        ArrayVerticalBox->AddChild(ElementRow);
    }
    
    return ExpandableArea;
}

UWidget* URapidPropertyEditor::CreateMapPropertyWidget(FMapProperty* MapProperty, void* PropertyValue)
{
    if (!MapProperty || !PropertyValue)
    {
        return nullptr;
    }
    
    // 创建一个可展开区域
    UExpandableArea* ExpandableArea = NewObject<UExpandableArea>(this);
    ExpandableArea->SetIsExpanded(false);
    
    // 创建一个垂直容器放入映射元素
    UVerticalBox* MapVerticalBox = NewObject<UVerticalBox>(this);
    
    // 获取映射
    FScriptMapHelper MapHelper(MapProperty, PropertyValue);
    int32 ElementCount = MapHelper.Num();
    
    // 创建映射标题
    UTextBlock* HeaderText = NewObject<UTextBlock>(this);
    HeaderText->SetText(FText::Format(FText::FromString("{0} [{1}]"), 
        FText::FromString(MapProperty->GetName()), 
        FText::AsNumber(ElementCount)));
    
    // ExpandableArea->SetHeaderContent(HeaderText);
    // ExpandableArea->SetContent(MapVerticalBox);
    
    // 获取键值类型
    FProperty* KeyProperty = MapProperty->KeyProp;
    FProperty* ValueProperty = MapProperty->ValueProp;
    
    // 显示已存在的映射对
    for (int32 i = 0; i < MapHelper.GetMaxIndex(); ++i)
    {
        if (!MapHelper.IsValidIndex(i))
        {
            continue;
        }
        
        UHorizontalBox* PairRow = NewObject<UHorizontalBox>(this);
        
        // 获取键值
        const uint8* KeyPtr = MapHelper.GetKeyPtr(i);
        const uint8* ValuePtr = MapHelper.GetValuePtr(i);
        
        // 创建键文本
        FString KeyString;
        // KeyProperty->ExportTextItem(KeyString, KeyPtr, nullptr, TargetObject, PPF_None);
        
        UTextBlock* KeyText = NewObject<UTextBlock>(this);
        KeyText->SetText(FText::Format(FText::FromString("[{0}]:"), FText::FromString(KeyString)));
        PairRow->AddChild(KeyText);
        
        // 创建值控件
        UWidget* ValueWidget = CreatePropertyWidget(ValueProperty, (void*)ValuePtr);
        
        if (ValueWidget)
        {
            // 设置映射值控件的属性名称（使用特殊格式以便识别映射元素）
            FName ValuePropertyName = *FString::Printf(TEXT("%s[%s]"), *MapProperty->GetName(), *KeyString);
            SetWidgetPropertyName(ValueWidget, ValuePropertyName);
            
            PairRow->AddChild(ValueWidget);
        }
        
        MapVerticalBox->AddChild(PairRow);
    }
    
    return ExpandableArea;
}

UWidget* URapidPropertyEditor::CreateStructPropertyWidget(FStructProperty* StructProperty, void* PropertyValue)
{
    if (!StructProperty || !PropertyValue)
    {
        return nullptr;
    }
    
    // 创建一个可展开区域
    UExpandableArea* ExpandableArea = NewObject<UExpandableArea>(this);
    ExpandableArea->SetIsExpanded(false);
    
    // 创建一个垂直容器放入结构体字段
    UVerticalBox* StructVerticalBox = NewObject<UVerticalBox>(this);
    
    // 创建结构体标题
    UTextBlock* HeaderText = NewObject<UTextBlock>(this);
    UScriptStruct* ScriptStruct = StructProperty->Struct;
    HeaderText->SetText(FText::FromString(ScriptStruct->GetName()));
    
    ExpandableArea->SetContentForSlot(TEXT("Header"), HeaderText);
    ExpandableArea->SetContentForSlot(TEXT("Body"), StructVerticalBox);
    
    // 遍历结构体的所有字段
    for (TFieldIterator<FProperty> It(ScriptStruct); It; ++It)
    {
        FProperty* StructField = *It;
        
        // 获取字段地址
        void* FieldValuePtr = StructField->ContainerPtrToValuePtr<void>(PropertyValue);
        
        // 创建字段控件
        UWidget* FieldWidget = CreatePropertyWidget(StructField, FieldValuePtr);
        
        if (FieldWidget)
        {
            // 设置结构体字段控件的属性名称（使用特殊格式以便识别结构体字段）
            FName FieldPropertyName = *FString::Printf(TEXT("%s.%s"), *StructProperty->GetName(), *StructField->GetName());
            SetWidgetPropertyName(FieldWidget, FieldPropertyName);
            
            // 创建字段行并添加到结构体容器
            FString FieldDisplayName = GetPropertyDisplayName(StructField);
            UWidget* FieldRow = CreatePropertyRow(FieldDisplayName, FieldWidget, 
                (StructField->IsA<FStructProperty>() || StructField->IsA<FArrayProperty>() || StructField->IsA<FMapProperty>()));
            
            StructVerticalBox->AddChild(FieldRow);
        }
    }
    
    return ExpandableArea;
}

UWidget* URapidPropertyEditor::CreatePropertyRow(const FString& PropertyName, UWidget* ValueWidget, bool bIsExpandable)
{
    // 创建水平布局
    UHorizontalBox* HorizontalBox = NewObject<UHorizontalBox>(this);
    
    // 添加属性名标签
    UTextBlock* PropertyNameText = NewObject<UTextBlock>(this);
    PropertyNameText->SetText(FText::FromString(PropertyName + ": "));
    
    // 添加到布局
    HorizontalBox->AddChild(PropertyNameText);
    
    // 添加值控件
    HorizontalBox->AddChild(ValueWidget);
    
    return HorizontalBox;
}

void URapidPropertyEditor::HandleFloatValueChanged(float NewValue)
{
    if (!TargetObject)
    {
        return;
    }
    
    // 获取触发事件的控件
    UWidget* Sender = Cast<UWidget>(GetOwningPlayerPawn()->GetInstigator());
    if (!Sender)
    {
        return;
    }

    // 获取属性名称
    FName PropertyName = GetPropertyNameFromWidget(Sender);
    if (PropertyName.IsNone())
    {
        return;
    }
    
    FProperty* Property = TargetObject->GetClass()->FindPropertyByName(PropertyName);
    
    if (Property)
    {
        if (Property->IsA<FFloatProperty>())
        {
            UpdatePropertyValue(PropertyName, &NewValue, Property);
        }
        else if (Property->IsA<FIntProperty>())
        {
            int32 IntValue = FMath::RoundToInt(NewValue);
            UpdatePropertyValue(PropertyName, &IntValue, Property);
        }
    }
}

void URapidPropertyEditor::HandleIntValueChanged(int32 NewValue)
{
    if (!TargetObject)
    {
        return;
    }
    
    // 获取触发事件的控件
    UWidget* Sender = Cast<UWidget>(GetOwningPlayerPawn()->GetInstigator());
    if (!Sender)
    {
        return;
    }

    // 获取属性名称
    FName PropertyName = GetPropertyNameFromWidget(Sender);
    if (PropertyName.IsNone())
    {
        return;
    }
    
    FProperty* Property = TargetObject->GetClass()->FindPropertyByName(PropertyName);
    
    if (Property && Property->IsA<FIntProperty>())
    {
        UpdatePropertyValue(PropertyName, &NewValue, Property);
    }
}

void URapidPropertyEditor::HandleStringValueChanged(const FText& NewText, ETextCommit::Type CommitType)
{
    if (!TargetObject || CommitType != ETextCommit::OnEnter)
    {
        return;
    }
    
    // 获取触发事件的控件
    UWidget* Sender = Cast<UWidget>(GetOwningPlayerPawn()->GetInstigator());
    if (!Sender)
    {
        return;
    }

    // 获取属性名称
    FName PropertyName = GetPropertyNameFromWidget(Sender);
    if (PropertyName.IsNone())
    {
        return;
    }
    
    FProperty* Property = TargetObject->GetClass()->FindPropertyByName(PropertyName);
    
    if (Property)
    {
        if (Property->IsA<FStrProperty>())
        {
            FString StringValue = NewText.ToString();
            UpdatePropertyValue(PropertyName, &StringValue, Property);
        }
        else if (Property->IsA<FNameProperty>())
        {
            FName NameValue = FName(*NewText.ToString());
            UpdatePropertyValue(PropertyName, &NameValue, Property);
        }
        else if (Property->IsA<FTextProperty>())
        {
            UpdatePropertyValue(PropertyName, &NewText, Property);
        }
    }
}

void URapidPropertyEditor::HandleBoolValueChanged(bool NewValue)
{
    if (!TargetObject)
    {
        return;
    }
    
    // 获取触发事件的控件
    UWidget* Sender = Cast<UWidget>(GetOwningPlayerPawn()->GetInstigator());
    if (!Sender)
    {
        return;
    }

    // 获取属性名称
    FName PropertyName = GetPropertyNameFromWidget(Sender);
    if (PropertyName.IsNone())
    {
        return;
    }
    
    FProperty* Property = TargetObject->GetClass()->FindPropertyByName(PropertyName);
    
    if (Property && Property->IsA<FBoolProperty>())
    {
        UpdatePropertyValue(PropertyName, &NewValue, Property);
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

FName URapidPropertyEditor::GetPropertyNameFromWidget(UWidget* Widget) const
{
    if (!Widget)
    {
        return NAME_None;
    }

    const FName* PropertyName = WidgetToPropertyMap.Find(Widget);
    if (PropertyName)
    {
        return *PropertyName;
    }

    return NAME_None;
}

void URapidPropertyEditor::SetWidgetPropertyName(UWidget* Widget, FName PropertyName)
{
    if (Widget && !PropertyName.IsNone())
    {
        WidgetToPropertyMap.Add(Widget, PropertyName);
    }
}

void URapidPropertyEditor::UpdatePropertyValue(FName PropertyName, const void* NewValue, FProperty* Property)
{
    if (!TargetObject || !NewValue)
    {
        return;
    }
    
    if (!Property)
    {
        Property = TargetObject->GetClass()->FindPropertyByName(PropertyName);
    }
    
    if (!Property)
    {
        return;
    }
    
    // 获取属性地址
    void* PropertyValuePtr = Property->ContainerPtrToValuePtr<void>(TargetObject);
    
    // 更新属性值
    if (Property->IsA<FFloatProperty>())
    {
        CastField<FFloatProperty>(Property)->SetPropertyValue(PropertyValuePtr, *((float*)NewValue));
    }
    else if (Property->IsA<FIntProperty>())
    {
        CastField<FIntProperty>(Property)->SetPropertyValue(PropertyValuePtr, *((int32*)NewValue));
    }
    else if (Property->IsA<FBoolProperty>())
    {
        CastField<FBoolProperty>(Property)->SetPropertyValue(PropertyValuePtr, *((bool*)NewValue));
    }
    else if (Property->IsA<FStrProperty>())
    {
        CastField<FStrProperty>(Property)->SetPropertyValue(PropertyValuePtr, *((FString*)NewValue));
    }
    else if (Property->IsA<FNameProperty>())
    {
        CastField<FNameProperty>(Property)->SetPropertyValue(PropertyValuePtr, *((FName*)NewValue));
    }
    else if (Property->IsA<FTextProperty>())
    {
        CastField<FTextProperty>(Property)->SetPropertyValue(PropertyValuePtr, *((FText*)NewValue));
    }
    
    // 通知属性改变
    NotifyPropertyChanged(PropertyName, Property);
}

void URapidPropertyEditor::NotifyPropertyChanged(FName PropertyName, FProperty* Property)
{
    // 广播属性改变事件
    // OnPropertyChanged.Broadcast(TargetObject, PropertyName, Property);
} 