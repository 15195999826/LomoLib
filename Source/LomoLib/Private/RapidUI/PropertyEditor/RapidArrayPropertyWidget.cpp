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
#include "RapidUI/PropertyEditor/RapidArrayElementWidget.h"

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

TSubclassOf<URapidArrayElementWidget> URapidArrayPropertyWidget::GetElementWidgetClass() const
{
    return ElementWidgetClass;
}

void URapidArrayPropertyWidget::SetElementWidgetClass(TSubclassOf<URapidArrayElementWidget> InElementWidgetClass)
{
    ElementWidgetClass = InElementWidgetClass;
    
    // 如果已有数据，则重新创建元素控件
    if (ArrayProperty && InnerProperty && ValuePtr)
    {
        CreateElementWidgets();
    }
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
    ElementUWidgets.Empty();
    
    // 获取数组辅助类
    FScriptArrayHelper ArrayHelper(ArrayProperty, ValuePtr);
    
    // 遍历数组元素创建控件
    for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
    {
        CreateElementWidget(Index);
    }
}

void URapidArrayPropertyWidget::CreateElementWidget(int32 ElementIndex)
{
    if (!ArrayProperty || !InnerProperty || !TargetObject)
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
    
    // 获取元素地址
    void* ElementValuePtr = ArrayHelper.GetRawPtr(ElementIndex);
    
    // 获取属性编辑器
    URapidPropertyEditor* PropertyEditor = Cast<URapidPropertyEditor>(GetTypedOuter<URapidPropertyEditor>());
    if (!PropertyEditor)
    {
        return;
    }
    
    // 创建元素属性控件
    URapidPropertyWidget* ElementPropertyWidget = PropertyEditor->CreatePropertyWidgetForType(this, InnerProperty);
    
    if (!ElementPropertyWidget)
    {
        return;
    }
    
    // 初始化元素控件
    ElementPropertyWidget->InitializePropertyWidget(TargetObject, InnerProperty, ElementValuePtr);
    
    // 设置元素名称为索引
    ElementPropertyWidget->SetPropertyName(*FString::Printf(TEXT("%s[%d]"), *PropertyName.ToString(), ElementIndex));
    ElementPropertyWidget->SetPropertyDisplayName(FText::AsNumber(ElementIndex));
    
    // 绑定值变化事件
    ElementPropertyWidget->OnPropertyValueChanged.AddDynamic(this, &URapidArrayPropertyWidget::HandleChildPropertyValueChanged);
    
    // 检查是否指定了元素小部件类
    check(ElementWidgetClass)
    // 创建自定义元素小部件
    URapidArrayElementWidget* ElementUWidget = CreateWidget<URapidArrayElementWidget>(this, ElementWidgetClass);
    ElementUWidget->InitializeElementWidget(TargetObject, InnerProperty, ElementValuePtr, ElementIndex);
            
    // 设置属性控件
    ElementUWidget->SetElementPropertyWidget(ElementPropertyWidget);
            
    // 绑定删除事件
    ElementUWidget->OnDeleteElementClicked.AddDynamic(this, &URapidArrayPropertyWidget::HandleElementDeleteClicked);
            
    // 添加到数组中
    ElementUWidgets.Add(ElementUWidget);
    
    // 添加到内容垂直框
    ContentVerticalBox->AddChild(ElementUWidget);
}

void URapidArrayPropertyWidget::UpdateElementWidgets()
{
    if (!ArrayProperty || !InnerProperty)
    {
        return;
    }
    
    // 检查数组元素数量是否发生变化
    FScriptArrayHelper ArrayHelper(ArrayProperty, ValuePtr);
    if (ArrayHelper.Num() != ElementUWidgets.Num())
    {
        // 数组大小改变，需要重新创建所有元素控件
        CreateElementWidgets();
        return;
    }
    
    // 更新现有控件的值
    for (int32 Index = 0; Index < ElementUWidgets.Num(); ++Index)
    {
        if (ElementUWidgets[Index])
        {
            ElementUWidgets[Index]->UpdateValue();
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
    CreateElementWidget(NewIndex);
    
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