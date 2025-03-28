 

#include "RapidUI/PropertyEditor/RapidArrayPropertyWidget.h"
#include "Components/TextBlock.h"
#include "Components/ExpandableArea.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "UObject/UnrealType.h"
#include "UObject/PropertyPortFlags.h"
#include "RapidUI/PropertyEditor/RapidPropertyEditor.h"

URapidArrayPropertyWidget::URapidArrayPropertyWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , ArrayProperty(nullptr)
    , InnerProperty(nullptr)
{
}

void URapidArrayPropertyWidget::InitializePropertyWidget(UObject* InObject, FProperty* InProperty, void* InValuePtr)
{
    // 先调用父类初始化
    Super::InitializePropertyWidget(InObject, InProperty, InValuePtr);
    
    // 保存数组属性引用
    ArrayProperty = CastField<FArrayProperty>(InProperty);
    if (!ArrayProperty)
    {
        return;
    }
    
    // 获取内部元素属性
    InnerProperty = ArrayProperty->Inner;
    if (!InnerProperty)
    {
        return;
    }
    
    // 设置标题文本
    if (TitleText)
    {
        TitleText->SetText(FText::FromString(GetPropertyDisplayName().ToString()));
    }
    
    // 绑定添加元素按钮点击事件
    if (AddElementButton)
    {
        AddElementButton->OnClicked.AddDynamic(this, &URapidArrayPropertyWidget::HandleAddElementClicked);
    }
    
    // 创建元素控件
    CreateElementWidgets();
}

void URapidArrayPropertyWidget::UpdateValue_Implementation()
{
    // 刷新元素控件显示
    UpdateElementWidgets();
}

void URapidArrayPropertyWidget::CreateElementWidgets()
{
    if (!ArrayProperty || !InnerProperty || !ContentVerticalBox)
    {
        return;
    }
    
    // 清除现有控件
    ContentVerticalBox->ClearChildren();
    ElementWidgets.Empty();
    
    // 获取数组辅助类
    FScriptArrayHelper ArrayHelper(ArrayProperty, ValuePtr);
    
    // 遍历数组元素创建控件
    for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
    {
        URapidPropertyWidget* ElementWidget = CreateElementWidget(Index);
        if (ElementWidget)
        {
            ElementWidgets.Add(ElementWidget);
            ContentVerticalBox->AddChild(ElementWidget);
        }
    }
}

URapidPropertyWidget* URapidArrayPropertyWidget::CreateElementWidget(int32 ElementIndex)
{
    if (!ArrayProperty || !InnerProperty || !TargetObject)
    {
        return nullptr;
    }
    
    // 获取数组辅助类
    FScriptArrayHelper ArrayHelper(ArrayProperty, ValuePtr);
    
    // 检查索引是否有效
    if (!ArrayHelper.IsValidIndex(ElementIndex))
    {
        return nullptr;
    }
    
    // 获取元素地址
    void* ElementValuePtr = ArrayHelper.GetRawPtr(ElementIndex);
    
    // 创建水平布局用于包含元素控件和删除按钮
    UHorizontalBox* ElementRow = NewObject<UHorizontalBox>(this);
    ElementRow->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    
    // 创建边框包含索引标签
    UBorder* IndexBorder = NewObject<UBorder>(this);
    IndexBorder->SetPadding(FMargin(5, 2));
    
    // 创建索引标签
    UTextBlock* IndexText = NewObject<UTextBlock>(this);
    IndexText->SetText(FText::AsNumber(ElementIndex));
    IndexBorder->SetContent(IndexText);
    
    // 添加索引标签到行
    ElementRow->AddChild(IndexBorder);
    
    // 创建元素内容容器
    UBorder* ElementBorder = NewObject<UBorder>(this);
    ElementBorder->SetPadding(FMargin(5, 2));
    
    // 获取属性编辑器
    URapidPropertyEditor* PropertyEditor = Cast<URapidPropertyEditor>(GetTypedOuter<URapidPropertyEditor>());
    if (!PropertyEditor)
    {
        return nullptr;
    }
    
    // 创建元素属性控件
    URapidPropertyWidget* ElementWidget = nullptr;
    
    // 根据内部属性类型创建控件
    if (InnerProperty->IsA<FFloatProperty>() && PropertyEditor->GetFloatPropertyWidgetClass())
    {
        ElementWidget = CreateWidget<URapidPropertyWidget>(this, PropertyEditor->GetFloatPropertyWidgetClass());
    }
    else if (InnerProperty->IsA<FIntProperty>() && PropertyEditor->GetIntPropertyWidgetClass())
    {
        ElementWidget = CreateWidget<URapidPropertyWidget>(this, PropertyEditor->GetIntPropertyWidgetClass());
    }
    else if (InnerProperty->IsA<FBoolProperty>() && PropertyEditor->GetBoolPropertyWidgetClass())
    {
        ElementWidget = CreateWidget<URapidPropertyWidget>(this, PropertyEditor->GetBoolPropertyWidgetClass());
    }
    else if ((InnerProperty->IsA<FStrProperty>() || InnerProperty->IsA<FNameProperty>() || InnerProperty->IsA<FTextProperty>()) && PropertyEditor->GetStringPropertyWidgetClass())
    {
        ElementWidget = CreateWidget<URapidPropertyWidget>(this, PropertyEditor->GetStringPropertyWidgetClass());
    }
    else if (InnerProperty->IsA<FStructProperty>() && PropertyEditor->GetStructPropertyWidgetClass())
    {
        ElementWidget = CreateWidget<URapidPropertyWidget>(this, PropertyEditor->GetStructPropertyWidgetClass());
    }
    else if (InnerProperty->IsA<FArrayProperty>() && PropertyEditor->GetArrayPropertyWidgetClass())
    {
        ElementWidget = CreateWidget<URapidPropertyWidget>(this, PropertyEditor->GetArrayPropertyWidgetClass());
    }
    else if (InnerProperty->IsA<FMapProperty>() && PropertyEditor->GetMapPropertyWidgetClass())
    {
        ElementWidget = CreateWidget<URapidPropertyWidget>(this, PropertyEditor->GetMapPropertyWidgetClass());
    }
    
    if (ElementWidget)
    {
        // 初始化元素控件
        ElementWidget->InitializePropertyWidget(TargetObject, InnerProperty, ElementValuePtr);
        
        // 设置元素名称为索引
        ElementWidget->SetPropertyName(*FString::Printf(TEXT("%s[%d]"), *PropertyName.ToString(), ElementIndex));
        ElementWidget->SetPropertyDisplayName(FText::AsNumber(ElementIndex));
        
        // 绑定值变化事件
        ElementWidget->OnPropertyValueChanged.AddDynamic(this, &URapidArrayPropertyWidget::HandleChildPropertyValueChanged);
        
        // 设置元素内容
        ElementBorder->SetContent(ElementWidget);
    }
    
    // 添加元素内容到行
    ElementRow->AddChild(ElementBorder);
    
    // 创建删除按钮
    UButton* DeleteButton = NewObject<UButton>(this);
    DeleteButton->SetToolTipText(FText::FromString(TEXT("删除元素")));
    
    // 绑定删除按钮事件
    FScriptDelegate DeleteDelegate;
    DeleteDelegate.BindUFunction(this, "HandleElementDeleteClicked", ElementIndex);
    DeleteButton->OnClicked.Add(DeleteDelegate);
    
    // 创建删除按钮文本
    UTextBlock* DeleteText = NewObject<UTextBlock>(this);
    DeleteText->SetText(FText::FromString(TEXT("X")));
    DeleteButton->SetContent(DeleteText);
    
    // 添加删除按钮到行
    ElementRow->AddChild(DeleteButton);
    
    return Cast<URapidPropertyWidget>(ElementRow);
}

void URapidArrayPropertyWidget::UpdateElementWidgets()
{
    if (!ArrayProperty || !InnerProperty)
    {
        return;
    }
    
    // 检查数组元素数量是否发生变化
    FScriptArrayHelper ArrayHelper(ArrayProperty, ValuePtr);
    if (ArrayHelper.Num() != ElementWidgets.Num())
    {
        // 数组大小改变，需要重新创建所有元素控件
        CreateElementWidgets();
        return;
    }
    
    // 更新现有控件的值
    for (int32 Index = 0; Index < ElementWidgets.Num(); ++Index)
    {
        if (ElementWidgets[Index])
        {
            ElementWidgets[Index]->UpdateValue();
        }
    }
}

void URapidArrayPropertyWidget::HandleAddElementClicked()
{
    if (!ArrayProperty || !InnerProperty || !ValuePtr)
    {
        return;
    }
    
    // 获取数组辅助类
    FScriptArrayHelper ArrayHelper(ArrayProperty, ValuePtr);
    
    // 添加新元素
    const int32 NewIndex = ArrayHelper.AddValue();
    
    // 初始化新元素为默认值
    uint8* NewElementPtr = ArrayHelper.GetRawPtr(NewIndex);
    InnerProperty->InitializeValue(NewElementPtr);
    
    // 创建新元素的控件
    URapidPropertyWidget* ElementWidget = CreateElementWidget(NewIndex);
    if (ElementWidget)
    {
        ElementWidgets.Add(ElementWidget);
        ContentVerticalBox->AddChild(ElementWidget);
    }
    
    // 通知值改变
    NotifyPropertyValueChanged();
}

void URapidArrayPropertyWidget::HandleElementDeleteClicked(int32 ElementIndex)
{
    if (!ArrayProperty || !InnerProperty || !ValuePtr)
    {
        return;
    }
    
    // 获取数组辅助类
    FScriptArrayHelper ArrayHelper(ArrayProperty, ValuePtr);
    
    // 检查索引是否有效
    if (!ArrayHelper.IsValidIndex(ElementIndex))
    {
        return;
    }
    
    // 删除元素
    ArrayHelper.RemoveValues(ElementIndex, 1);
    
    // 重新创建所有元素控件
    CreateElementWidgets();
    
    // 通知值改变
    NotifyPropertyValueChanged();
}

void URapidArrayPropertyWidget::HandleChildPropertyValueChanged(UObject* Object, FName InPropertyName, URapidPropertyWidget* PropertyWidget)
{
    // 转发子属性改变事件
    NotifyPropertyValueChanged();
}