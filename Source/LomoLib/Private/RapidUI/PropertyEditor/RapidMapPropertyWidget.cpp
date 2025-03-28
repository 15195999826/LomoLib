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
    PairWidgets.Empty();

    // 获取映射辅助类
    FScriptMapHelper MapHelper(MapProperty, ValuePtr);

    // 遍历映射元素创建控件
    for (int32 Index = 0; Index < MapHelper.Num(); ++Index)
    {
        URapidPropertyWidget* PairWidget = CreatePairWidget(Index);
        if (PairWidget)
        {
            PairWidgets.Add(PairWidget);
            ContentVerticalBox->AddChild(PairWidget);
        }
    }
}

URapidPropertyWidget* URapidMapPropertyWidget::CreatePairWidget(int32 PairIndex)
{
    if (!MapProperty || !KeyProperty || !ValueProperty || !TargetObject)
    {
        return nullptr;
    }

    // 获取映射辅助类
    FScriptMapHelper MapHelper(MapProperty, ValuePtr);

    // 检查索引是否有效
    if (PairIndex < 0 || PairIndex >= MapHelper.Num())
    {
        return nullptr;
    }

    // 获取有效的对位索引
    int32 RealIndex = 0;
    for (int32 SparseIndex = 0; SparseIndex < MapHelper.GetMaxIndex(); ++SparseIndex)
    {
        if (MapHelper.IsValidIndex(SparseIndex))
        {
            if (RealIndex == PairIndex)
            {
                // 创建垂直布局用于包含键值对
                UVerticalBox* PairBox = NewObject<UVerticalBox>(this);
                PairBox->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

                // 创建边框
                UBorder* PairBorder = NewObject<UBorder>(this);
                PairBorder->SetPadding(FMargin(5));

                // 创建水平布局容器
                UHorizontalBox* HeaderBox = NewObject<UHorizontalBox>(this);

                // 添加索引标签
                UTextBlock* IndexText = NewObject<UTextBlock>(this);
                IndexText->SetText(FText::AsNumber(PairIndex));
                HeaderBox->AddChild(IndexText);

                // 添加删除按钮
                UButton* DeleteButton = NewObject<UButton>(this);
                DeleteButton->SetToolTipText(FText::FromString(TEXT("删除元素")));

                // 绑定删除按钮事件
                FScriptDelegate DeleteDelegate;
                DeleteDelegate.BindUFunction(this, "HandleElementDeleteClicked");
                DeleteButton->OnClicked.Add(DeleteDelegate);

                // 创建删除按钮文本
                UTextBlock* DeleteText = NewObject<UTextBlock>(this);
                DeleteText->SetText(FText::FromString(TEXT("X")));
                DeleteButton->SetContent(DeleteText);

                // 添加删除按钮
                HeaderBox->AddChild(DeleteButton);

                // 添加头部到边框
                PairBorder->SetContent(HeaderBox);

                // 添加边框到容器
                PairBox->AddChild(PairBorder);

                // 获取元素地址
                uint8* PairData = MapHelper.GetPairPtr(SparseIndex);
                uint8* KeyData = PairData;
                uint8* ValueData = PairData + KeyProperty->GetSize();

                // 获取属性编辑器
                URapidPropertyEditor* PropertyEditor = Cast<URapidPropertyEditor>(GetTypedOuter<URapidPropertyEditor>());
                if (!PropertyEditor)
                {
                    return nullptr;
                }

                // 创建键控件
                UHorizontalBox* KeyBox = NewObject<UHorizontalBox>(this);

                UTextBlock* KeyLabel = NewObject<UTextBlock>(this);
                KeyLabel->SetText(FText::FromString(TEXT("键:")));
                KeyBox->AddChild(KeyLabel);

                // 使用辅助函数创建键控件
                URapidPropertyWidget* KeyWidget = PropertyEditor->CreatePropertyWidgetForType(this, KeyProperty);

                if (KeyWidget)
                {
                    // 初始化键控件
                    KeyWidget->InitializePropertyWidget(TargetObject, KeyProperty, KeyData);

                    // 设置键名称
                    KeyWidget->SetPropertyName(*FString::Printf(TEXT("%s[%d].Key"), *PropertyName.ToString(), PairIndex));
                    KeyWidget->SetPropertyDisplayName(FText::FromString(TEXT("键")));

                    // 绑定值变化事件
                    KeyWidget->OnPropertyValueChanged.AddDynamic(this, &URapidMapPropertyWidget::HandleChildPropertyValueChanged);

                    // 添加键控件
                    KeyBox->AddChild(KeyWidget);
                }

                // 添加键行
                PairBox->AddChild(KeyBox);

                // 创建值控件
                UHorizontalBox* ValueBox = NewObject<UHorizontalBox>(this);

                UTextBlock* ValueLabel = NewObject<UTextBlock>(this);
                ValueLabel->SetText(FText::FromString(TEXT("值:")));
                ValueBox->AddChild(ValueLabel);

                // 使用辅助函数创建值控件
                URapidPropertyWidget* ValueWidget = PropertyEditor->CreatePropertyWidgetForType(this, ValueProperty);

                if (ValueWidget)
                {
                    // 初始化值控件
                    ValueWidget->InitializePropertyWidget(TargetObject, ValueProperty, ValueData);

                    // 设置值名称
                    ValueWidget->SetPropertyName(*FString::Printf(TEXT("%s[%d].Value"), *PropertyName.ToString(), PairIndex));
                    ValueWidget->SetPropertyDisplayName(FText::FromString(TEXT("值")));

                    // 绑定值变化事件
                    ValueWidget->OnPropertyValueChanged.AddDynamic(this, &URapidMapPropertyWidget::HandleChildPropertyValueChanged);

                    // 添加值控件
                    ValueBox->AddChild(ValueWidget);
                }

                // 添加值行
                PairBox->AddChild(ValueBox);

                return Cast<URapidPropertyWidget>(PairBox);
            }

            RealIndex++;
        }
    }

    return nullptr;
}

void URapidMapPropertyWidget::UpdatePairWidgets()
{
    if (!MapProperty || !KeyProperty || !ValueProperty)
    {
        return;
    }

    // 检查映射元素数量是否发生变化
    FScriptMapHelper MapHelper(MapProperty, ValuePtr);
    if (MapHelper.Num() != PairWidgets.Num())
    {
        // 映射大小改变，需要重新创建所有元素控件
        CreatePairWidgets();
        return;
    }

    // 更新现有控件的值
    for (int32 Index = 0; Index < PairWidgets.Num(); ++Index)
    {
        if (PairWidgets[Index])
        {
            PairWidgets[Index]->UpdateValue();
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