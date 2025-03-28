#include "RapidUI/PropertyEditor/RapidMapPropertyWidget.h"
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
#include "RapidUI/PropertyEditor/RapidMapElementWidget.h"

URapidMapPropertyWidget::URapidMapPropertyWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , MapProperty(nullptr)
    , KeyProperty(nullptr)
    , ValueProperty(nullptr)
{
}

void URapidMapPropertyWidget::InitializePropertyWidget(UObject* InObject, FProperty* InProperty, void* InValuePtr)
{
    // 先调用父类初始化
    Super::InitializePropertyWidget(InObject, InProperty, InValuePtr);

    // 保存映射属性引用
    MapProperty = CastField<FMapProperty>(InProperty);
    if (!MapProperty)
    {
        return;
    }

    // 获取键和值属性
    KeyProperty = MapProperty->KeyProp;
    ValueProperty = MapProperty->ValueProp;

    if (!KeyProperty || !ValueProperty)
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
        AddElementButton->OnClicked.AddDynamic(this, &URapidMapPropertyWidget::HandleAddElementClicked);
    }

    // 创建元素控件
    CreatePairWidgets();
}

TSubclassOf<URapidMapElementWidget> URapidMapPropertyWidget::GetElementWidgetClass() const
{
    return ElementWidgetClass;
}

void URapidMapPropertyWidget::SetElementWidgetClass(TSubclassOf<URapidMapElementWidget> InElementWidgetClass)
{
    ElementWidgetClass = InElementWidgetClass;
    
    // 如果已有数据，则重新创建元素控件
    if (MapProperty && KeyProperty && ValueProperty && ValuePtr)
    {
        CreatePairWidgets();
    }
}

void URapidMapPropertyWidget::UpdateValue_Implementation()
{
    // 刷新元素控件显示
    UpdatePairWidgets();
}

void URapidMapPropertyWidget::CreatePairWidgets()
{
    if (!MapProperty || !KeyProperty || !ValueProperty || !ContentVerticalBox)
    {
        return;
    }

    // 清除现有控件
    ContentVerticalBox->ClearChildren();
    ElementUWidgets.Empty();

    // 获取映射辅助类
    FScriptMapHelper MapHelper(MapProperty, ValuePtr);

    // 遍历映射元素创建控件
    for (int32 Index = 0; Index < MapHelper.Num(); ++Index)
    {
        CreatePairWidget(Index);
    }
}

void URapidMapPropertyWidget::CreatePairWidget(int32 PairIndex)
{
    if (!MapProperty || !KeyProperty || !ValueProperty || !TargetObject)
    {
        return;
    }

    // 获取映射辅助类
    FScriptMapHelper MapHelper(MapProperty, ValuePtr);

    // 检查索引是否有效
    if (PairIndex < 0 || PairIndex >= MapHelper.Num())
    {
        return;
    }

    // 获取有效的对位索引
    int32 RealIndex = 0;
    int32 SparseIndex = -1;
    for (int32 i = 0; i < MapHelper.GetMaxIndex(); ++i)
    {
        if (MapHelper.IsValidIndex(i))
        {
            if (RealIndex == PairIndex)
            {
                SparseIndex = i;
                break;
            }
            RealIndex++;
        }
    }

    if (SparseIndex == -1)
    {
        return;
    }

    // 获取元素地址
    uint8* PairData = MapHelper.GetPairPtr(SparseIndex);
    uint8* KeyData = PairData;
    uint8* ValueData = PairData + KeyProperty->GetSize();

    // 获取属性编辑器
    URapidPropertyEditor* PropertyEditor = Cast<URapidPropertyEditor>(GetTypedOuter<URapidPropertyEditor>());
    if (!PropertyEditor)
    {
        return;
    }

    // 创建键控件
    URapidPropertyWidget* KeyWidget = PropertyEditor->CreatePropertyWidgetForType(this, KeyProperty);
    if (!KeyWidget)
    {
        return;
    }

    // 初始化键控件
    KeyWidget->InitializePropertyWidget(TargetObject, KeyProperty, KeyData);
    KeyWidget->SetPropertyName(*FString::Printf(TEXT("%s[%d].Key"), *PropertyName.ToString(), PairIndex));
    KeyWidget->SetPropertyDisplayName(FText::FromString(TEXT("键")));
    KeyWidget->OnPropertyValueChanged.AddDynamic(this, &URapidMapPropertyWidget::HandleChildPropertyValueChanged);

    // 创建值控件
    URapidPropertyWidget* ValueWidget = PropertyEditor->CreatePropertyWidgetForType(this, ValueProperty);
    if (!ValueWidget)
    {
        return;
    }

    // 初始化值控件
    ValueWidget->InitializePropertyWidget(TargetObject, ValueProperty, ValueData);
    ValueWidget->SetPropertyName(*FString::Printf(TEXT("%s[%d].Value"), *PropertyName.ToString(), PairIndex));
    ValueWidget->SetPropertyDisplayName(FText::FromString(TEXT("值")));
    ValueWidget->OnPropertyValueChanged.AddDynamic(this, &URapidMapPropertyWidget::HandleChildPropertyValueChanged);

    check(ElementWidgetClass)
    // 创建自定义元素小部件
    URapidMapElementWidget* ElementUWidget = CreateWidget<URapidMapElementWidget>(this, ElementWidgetClass);
    // 初始化元素小部件
    ElementUWidget->InitializeElementWidget(TargetObject, KeyProperty, ValueProperty, KeyData, ValueData, PairIndex);
            
    // 设置键值属性控件
    ElementUWidget->SetKeyPropertyWidget(KeyWidget);
    ElementUWidget->SetValuePropertyWidget(ValueWidget);
            
    // 绑定删除事件
    ElementUWidget->OnDeletePairClicked.AddDynamic(this, &URapidMapPropertyWidget::HandleElementDeleteClicked);
            
    // 添加到数组中
    ElementUWidgets.Add(ElementUWidget);
        
    // 添加到内容垂直框
    ContentVerticalBox->AddChild(ElementUWidget);
}

void URapidMapPropertyWidget::UpdatePairWidgets()
{
    if (!MapProperty || !KeyProperty || !ValueProperty)
    {
        return;
    }

    // 检查映射元素数量是否发生变化
    FScriptMapHelper MapHelper(MapProperty, ValuePtr);
    if (MapHelper.Num() != ElementUWidgets.Num())
    {
        // 映射大小改变，需要重新创建所有元素控件
        CreatePairWidgets();
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

void URapidMapPropertyWidget::HandleAddElementClicked()
{
    if (!MapProperty || !KeyProperty || !ValueProperty || !ValuePtr)
    {
        return;
    }

    // 获取映射辅助类
    FScriptMapHelper MapHelper(MapProperty, ValuePtr);

    // 添加新元素的索引
    const int32 NewPairIndex = MapHelper.AddDefaultValue_Invalid_NeedsRehash();

    // 获取添加的元素地址
    if (NewPairIndex != INDEX_NONE)
    {
        // 获取元素地址
        uint8* PairPtr = MapHelper.GetPairPtr(NewPairIndex);

        // 初始化键和值为默认值
        KeyProperty->InitializeValue(PairPtr);
        ValueProperty->InitializeValue(PairPtr + KeyProperty->GetSize());

        // 刷新哈希
        MapHelper.Rehash();

        // 重新创建所有元素控件
        CreatePairWidgets();

        // 通知值改变
        NotifyPropertyValueChanged();
    }
}

void URapidMapPropertyWidget::HandleElementDeleteClicked(int32 ElementIndex)
{
    if (!MapProperty || !KeyProperty || !ValueProperty || !ValuePtr)
    {
        return;
    }

    // 获取映射辅助类
    FScriptMapHelper MapHelper(MapProperty, ValuePtr);

    // 找到实际的稀疏索引
    int32 RealIndex = 0;
    for (int32 SparseIndex = 0; SparseIndex < MapHelper.GetMaxIndex(); ++SparseIndex)
    {
        if (MapHelper.IsValidIndex(SparseIndex))
        {
            if (RealIndex == ElementIndex)
            {
                // 删除元素
                MapHelper.RemoveAt(SparseIndex);

                // 重新创建所有元素控件
                CreatePairWidgets();

                // 通知值改变
                NotifyPropertyValueChanged();
                return;
            }

            RealIndex++;
        }
    }
}

void URapidMapPropertyWidget::HandleChildPropertyValueChanged(UObject* Object, FName InPropertyName, URapidPropertyWidget* PropertyWidget)
{
    // 转发子属性改变事件
    NotifyPropertyValueChanged();
}