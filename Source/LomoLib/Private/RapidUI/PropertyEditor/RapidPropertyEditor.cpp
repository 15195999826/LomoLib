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
#include "Components/ComboBoxString.h"
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

void URapidPropertyEditor::SetWidgetTemplates(TSubclassOf<UUserWidget> InLabelClass, 
                                           TSubclassOf<USpinBox> InFloatSpinBoxClass,
                                           TSubclassOf<USpinBox> InIntSpinBoxClass,
                                           TSubclassOf<UEditableTextBox> InTextBoxClass,
                                           TSubclassOf<UCheckBox> InCheckBoxClass,
                                           TSubclassOf<UComboBoxString> InComboBoxClass)
{
    LabelTemplateClass = InLabelClass;
    FloatSpinBoxTemplateClass = InFloatSpinBoxClass;
    IntSpinBoxTemplateClass = InIntSpinBoxClass;
    TextBoxTemplateClass = InTextBoxClass;
    CheckBoxTemplateClass = InCheckBoxClass;
    ComboBoxTemplateClass = InComboBoxClass;
    
    // 如果当前有对象，刷新显示
    if (TargetObject)
    {
        Refresh();
    }
}

template<class WidgetClass>
WidgetClass* URapidPropertyEditor::CreateWidgetFromTemplate(TSubclassOf<WidgetClass> TemplateClass)
{
    if (TemplateClass)
    {
        // 使用模板类创建控件
        return CreateWidget<WidgetClass>(this, TemplateClass);
    }
    else
    {
        // 如果没有提供模板，使用默认方式创建
        return NewObject<WidgetClass>(this);
    }
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
    
    // 增加对枚举类型的支持
    if (Property->IsA<FEnumProperty>())
    {
        return CreateEnumPropertyWidget(CastField<FEnumProperty>(Property), PropertyValue);
    }
    
    // 旧版UE中，枚举类型通常用FByteProperty表示
    if (Property->IsA<FByteProperty>() && CastField<FByteProperty>(Property)->IsEnum())
    {
        return CreateBytePropertyWidget(CastField<FByteProperty>(Property), PropertyValue);
    }

    return CreateBasicPropertyWidget(Property, PropertyValue);
}

UWidget* URapidPropertyEditor::CreateBasicPropertyWidget(FProperty* Property, void* PropertyValue)
{
    if (Property->IsA<FFloatProperty>())
    {
        // 创建Float编辑控件
        USpinBox* SpinBox = CreateWidgetFromTemplate<USpinBox>(FloatSpinBoxTemplateClass);
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
        USpinBox* SpinBox = CreateWidgetFromTemplate<USpinBox>(IntSpinBoxTemplateClass);
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
        UCheckBox* CheckBox = CreateWidgetFromTemplate<UCheckBox>(CheckBoxTemplateClass);
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
        UEditableTextBox* TextBox = CreateWidgetFromTemplate<UEditableTextBox>(TextBoxTemplateClass);
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
        UEditableTextBox* TextBox = CreateWidgetFromTemplate<UEditableTextBox>(TextBoxTemplateClass);
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
        UEditableTextBox* TextBox = CreateWidgetFromTemplate<UEditableTextBox>(TextBoxTemplateClass);
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
    // Property->ExportTextItem(ValueStr, PropertyValue, nullptr, TargetObject, PPF_None);
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
    UTextBlock* PropertyNameText;
    
    if (LabelTemplateClass)
    {
        // 使用模板创建标签
        UUserWidget* LabelWidget = CreateWidget<UUserWidget>(this, LabelTemplateClass);
        UTextBlock* TextBlock = Cast<UTextBlock>(LabelWidget->GetWidgetFromName(TEXT("Label")));
        if (TextBlock)
        {
            TextBlock->SetText(FText::FromString(PropertyName + ": "));
        }
        PropertyNameText = TextBlock;
        HorizontalBox->AddChild(LabelWidget);
    }
    else
    {
        // 使用默认方式创建标签
        PropertyNameText = NewObject<UTextBlock>(this);
        PropertyNameText->SetText(FText::FromString(PropertyName + ": "));
        HorizontalBox->AddChild(PropertyNameText);
    }
    
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

UWidget* URapidPropertyEditor::CreateEnumPropertyWidget(FEnumProperty* EnumProperty, void* PropertyValue)
{
    if (!EnumProperty || !PropertyValue)
    {
        return nullptr;
    }
    
    // 创建下拉框控件
    UComboBoxString* ComboBox = CreateWidgetFromTemplate<UComboBoxString>(ComboBoxTemplateClass);
    
    // 获取枚举类型
    UEnum* Enum = EnumProperty->GetEnum();
    
    // 获取当前枚举值
    FNumericProperty* UnderlyingProperty = EnumProperty->GetUnderlyingProperty();
    int64 EnumValue = UnderlyingProperty->GetSignedIntPropertyValue(PropertyValue);
    
    // 填充所有枚举项
    int32 EnumCount = Enum->NumEnums();
    FString CurrentSelection;
    
    for (int32 i = 0; i < EnumCount; i++)
    {
        // 跳过隐藏的枚举项(比如以UMETA(Hidden)标记的项)
        if (Enum->HasMetaData(TEXT("Hidden"), i))
        {
            continue;
        }
        
        // 获取枚举的值和名称
        int64 Value = Enum->GetValueByIndex(i);
        FString DisplayName = Enum->GetDisplayNameTextByIndex(i).ToString();
        if (DisplayName.IsEmpty())
        {
            DisplayName = Enum->GetNameStringByIndex(i);
        }
        
        // 将枚举项添加到下拉框
        ComboBox->AddOption(DisplayName);
        
        // 如果是当前值，设置为选中项
        if (Value == EnumValue)
        {
            CurrentSelection = DisplayName;
        }
    }
    
    // 设置当前选中项
    ComboBox->SetSelectedOption(CurrentSelection);
    
    // 绑定选择变化事件
    ComboBox->OnSelectionChanged.AddDynamic(this, &URapidPropertyEditor::HandleEnumValueChanged);
    
    return ComboBox;
}

UWidget* URapidPropertyEditor::CreateBytePropertyWidget(FByteProperty* ByteProperty, void* PropertyValue)
{
    if (!ByteProperty || !PropertyValue || !ByteProperty->IsEnum())
    {
        return nullptr;
    }
    
    // 创建下拉框控件
    UComboBoxString* ComboBox = CreateWidgetFromTemplate<UComboBoxString>(ComboBoxTemplateClass);
    
    // 获取枚举类型
    UEnum* Enum = ByteProperty->Enum;
    
    // 获取当前枚举值
    uint8 EnumValue = ByteProperty->GetPropertyValue(PropertyValue);
    
    // 填充所有枚举项
    int32 EnumCount = Enum->NumEnums();
    FString CurrentSelection;
    
    for (int32 i = 0; i < EnumCount; i++)
    {
        // 跳过隐藏的枚举项
        if (Enum->HasMetaData(TEXT("Hidden"), i))
        {
            continue;
        }
        
        // 获取枚举的值和名称
        int64 Value = Enum->GetValueByIndex(i);
        FString DisplayName = Enum->GetDisplayNameTextByIndex(i).ToString();
        if (DisplayName.IsEmpty())
        {
            DisplayName = Enum->GetNameStringByIndex(i);
        }
        
        // 将枚举项添加到下拉框
        ComboBox->AddOption(DisplayName);
        
        // 如果是当前值，设置为选中项
        if (Value == EnumValue)
        {
            CurrentSelection = DisplayName;
        }
    }
    
    // 设置当前选中项
    ComboBox->SetSelectedOption(CurrentSelection);
    
    // 绑定选择变化事件
    ComboBox->OnSelectionChanged.AddDynamic(this, &URapidPropertyEditor::HandleEnumValueChanged);
    
    return ComboBox;
}

void URapidPropertyEditor::HandleEnumValueChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    // if (!TargetObject || SelectionType != ESelectInfo::OnSelectionChanged)
    // {
    //     return;
    // }
    //
    // // 获取触发事件的控件
    // UWidget* Sender = Cast<UWidget>(GetOwningPlayerPawn()->GetInstigator());
    // if (!Sender)
    // {
    //     return;
    // }
    //
    // // 获取属性名称
    // FName PropertyName = GetPropertyNameFromWidget(Sender);
    // if (PropertyName.IsNone())
    // {
    //     return;
    // }
    //
    // FProperty* Property = TargetObject->GetClass()->FindPropertyByName(PropertyName);
    // if (!Property)
    // {
    //     return;
    // }
    //
    // UEnum* Enum = nullptr;
    // int64 CurrentValue = 0;
    //
    // // 获取枚举对象和当前值
    // if (Property->IsA<FEnumProperty>())
    // {
    //     FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property);
    //     Enum = EnumProperty->GetEnum();
    //     
    //     // 获取当前枚举值
    //     void* PropertyValuePtr = Property->ContainerPtrToValuePtr<void>(TargetObject);
    //     FNumericProperty* UnderlyingProperty = EnumProperty->GetUnderlyingProperty();
    //     CurrentValue = UnderlyingProperty->GetSignedIntPropertyValue(PropertyValuePtr);
    // }
    // else if (Property->IsA<FByteProperty>() && CastField<FByteProperty>(Property)->IsEnum())
    // {
    //     FByteProperty* ByteProperty = CastField<FByteProperty>(Property);
    //     Enum = ByteProperty->Enum;
    //     
    //     // 获取当前枚举值
    //     void* PropertyValuePtr = Property->ContainerPtrToValuePtr<void>(TargetObject);
    //     CurrentValue = ByteProperty->GetPropertyValue(PropertyValuePtr);
    // }
    //
    // if (Enum)
    // {
    //     // 查找选择的枚举值
    //     for (int32 i = 0; i < Enum->NumEnums(); i++)
    //     {
    //         FString DisplayName = Enum->GetDisplayNameTextByIndex(i).ToString();
    //         if (DisplayName.IsEmpty())
    //         {
    //             DisplayName = Enum->GetNameStringByIndex(i);
    //         }
    //         
    //         if (DisplayName == SelectedItem)
    //         {
    //             // 找到了选择的枚举值，更新属性
    //             int64 NewValue = Enum->GetValueByIndex(i);
    //             
    //             if (Property->IsA<FEnumProperty>())
    //             {
    //                 FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property);
    //                 void* PropertyValuePtr = Property->ContainerPtrToValuePtr<void>(TargetObject);
    //                 FNumericProperty* UnderlyingProperty = EnumProperty->GetUnderlyingProperty();
    //                 
    //                 if (UnderlyingProperty->IsA<FByteProperty>())
    //                 {
    //                     uint8 Value = static_cast<uint8>(NewValue);
    //                     UnderlyingProperty->SetIntPropertyValue(PropertyValuePtr, Value);
    //                 }
    //                 else if (UnderlyingProperty->IsA<FIntProperty>())
    //                 {
    //                     int32 Value = static_cast<int32>(NewValue);
    //                     UnderlyingProperty->SetIntPropertyValue(PropertyValuePtr, Value);
    //                 }
    //                 else if (UnderlyingProperty->IsA<FInt64Property>())
    //                 {
    //                     UnderlyingProperty->SetIntPropertyValue(PropertyValuePtr, NewValue);
    //                 }
    //             }
    //             else if (Property->IsA<FByteProperty>())
    //             {
    //                 FByteProperty* ByteProperty = CastField<FByteProperty>(Property);
    //                 void* PropertyValuePtr = Property->ContainerPtrToValuePtr<void>(TargetObject);
    //                 uint8 Value = static_cast<uint8>(NewValue);
    //                 ByteProperty->SetPropertyValue(PropertyValuePtr, Value);
    //             }
    //             
    //             // 通知属性改变
    //             NotifyPropertyChanged(PropertyName, Property);
    //             break;
    //         }
    //     }
    // }
} 