#include "RapidUI/PropertyEditor/RapidMapElementWidget.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "UObject/UnrealType.h"

URapidMapElementWidget::URapidMapElementWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , PairIndex(INDEX_NONE)
    , KeyPropertyWidget(nullptr)
    , ValuePropertyWidget(nullptr)
{
}

void URapidMapElementWidget::InitializeElementWidget(UObject* InObject, FProperty* InKeyProperty, FProperty* InValueProperty, 
                                                   void* InKeyValuePtr, void* InValueValuePtr, int32 InPairIndex)
{
    PairIndex = InPairIndex;
    
    // 设置索引文本
    if (IndexText)
    {
        IndexText->SetText(FText::AsNumber(PairIndex));
    }
    
    // 设置删除按钮点击事件
    if (DeleteButton)
    {
        DeleteButton->OnClicked.AddDynamic(this, &URapidMapElementWidget::HandleDeleteButtonClicked);
    }
}

void URapidMapElementWidget::UpdateValue_Implementation()
{
    // 更新键和值属性控件
    if (KeyPropertyWidget)
    {
        KeyPropertyWidget->UpdateValue();
    }
    
    if (ValuePropertyWidget)
    {
        ValuePropertyWidget->UpdateValue();
    }
}

URapidPropertyWidget* URapidMapElementWidget::GetKeyPropertyWidget() const
{
    return KeyPropertyWidget;
}

void URapidMapElementWidget::SetKeyPropertyWidget(URapidPropertyWidget* InPropertyWidget)
{
    KeyPropertyWidget = InPropertyWidget;
    
    // 将键属性控件添加到容器中
    if (KeyContainer && KeyPropertyWidget)
    {
        KeyContainer->SetContent(KeyPropertyWidget);
    }
}

URapidPropertyWidget* URapidMapElementWidget::GetValuePropertyWidget() const
{
    return ValuePropertyWidget;
}

void URapidMapElementWidget::SetValuePropertyWidget(URapidPropertyWidget* InPropertyWidget)
{
    ValuePropertyWidget = InPropertyWidget;
    
    // 将值属性控件添加到容器中
    if (ValueContainer && ValuePropertyWidget)
    {
        ValueContainer->SetContent(ValuePropertyWidget);
    }
}

int32 URapidMapElementWidget::GetPairIndex() const
{
    return PairIndex;
}

void URapidMapElementWidget::HandleDeleteButtonClicked()
{
    // 触发删除元素事件
    OnDeletePairClicked.Broadcast(PairIndex);
}

void URapidMapElementWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 绑定初始事件 - 在蓝图构造后执行
    if (DeleteButton)
    {
        DeleteButton->OnClicked.AddDynamic(this, &URapidMapElementWidget::HandleDeleteButtonClicked);
    }
} 