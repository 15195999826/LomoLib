#include "RapidUI/PropertyEditor/RapidArrayElementWidget.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "UObject/UnrealType.h"

URapidArrayElementWidget::URapidArrayElementWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , ElementIndex(INDEX_NONE)
    , ElementPropertyWidget(nullptr)
{
}

void URapidArrayElementWidget::InitializeElementWidget(UObject* InObject, FProperty* InElementProperty, void* InElementValuePtr, int32 InElementIndex)
{
    ElementIndex = InElementIndex;
    
    // 设置索引文本
    if (IndexText)
    {
        IndexText->SetText(FText::AsNumber(ElementIndex));
    }
    
    // 设置删除按钮点击事件
    if (DeleteButton)
    {
        DeleteButton->OnClicked.AddDynamic(this, &URapidArrayElementWidget::HandleDeleteButtonClicked);
    }
}

void URapidArrayElementWidget::UpdateValue_Implementation()
{
    // 更新元素属性控件
    if (ElementPropertyWidget)
    {
        ElementPropertyWidget->UpdateValue();
    }
}

URapidPropertyWidget* URapidArrayElementWidget::GetElementPropertyWidget() const
{
    return ElementPropertyWidget;
}

void URapidArrayElementWidget::SetElementPropertyWidget(URapidPropertyWidget* InPropertyWidget)
{
    ElementPropertyWidget = InPropertyWidget;
    
    // 将属性控件添加到容器中
    if (ElementContainer && ElementPropertyWidget)
    {
        ElementContainer->SetContent(ElementPropertyWidget);
    }
}

int32 URapidArrayElementWidget::GetElementIndex() const
{
    return ElementIndex;
}

void URapidArrayElementWidget::HandleDeleteButtonClicked()
{
    // 触发删除元素事件
    OnDeleteElementClicked.Broadcast(ElementIndex);
}

void URapidArrayElementWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 绑定初始事件 - 在蓝图构造后执行
    if (DeleteButton)
    {
        DeleteButton->OnClicked.AddDynamic(this, &URapidArrayElementWidget::HandleDeleteButtonClicked);
    }
} 