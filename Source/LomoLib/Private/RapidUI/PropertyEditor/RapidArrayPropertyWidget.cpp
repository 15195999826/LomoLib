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

bool URapidArrayPropertyWidget::InitializePropertyWidget(UObject* InObject, FProperty* InProperty, const FName& InPropertyName)
{
    // 调用父类的初始化方法
    if (!Super::InitializePropertyWidget(InObject, InProperty, InPropertyName))
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidArrayPropertyWidget初始化失败: 父类初始化返回false"));
        return false;
    }

    // 参数校验
    if (!InObject || !InProperty)
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidArrayPropertyWidget初始化失败: 对象或属性为空"));
        return false;
    }
    
    // 保存数组属性引用
    ArrayProperty = CastField<FArrayProperty>(InProperty);
    if (!ArrayProperty)
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidArrayPropertyWidget初始化失败: 属性类型不是Array"));
        return false;
    }
    
    // 获取内部元素属性
    InnerProperty = ArrayProperty->Inner;
    if (!InnerProperty)
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidArrayPropertyWidget初始化失败: 数组内部元素类型为空"));
        return false;
    }
    
    // 设置标题文本
    if (TitleText)
    {
        TitleText->SetText(PropertyDisplayName);
    }
    
    // 绑定添加元素按钮点击事件
    if (AddElementButton)
    {
        AddElementButton->OnClicked.AddDynamic(this, &URapidArrayPropertyWidget::HandleAddElementClicked);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidArrayPropertyWidget初始化失败: 添加元素按钮为空"));
        return false;
    }
    
    // 创建元素控件
    CreateElementWidgets();
    
    return true;
}

TSubclassOf<URapidArrayElementWidget> URapidArrayPropertyWidget::GetElementWidgetClass() const
{
    return ElementWidgetClass;
}

void URapidArrayPropertyWidget::SetElementWidgetClass(TSubclassOf<URapidArrayElementWidget> InElementWidgetClass)
{
    ElementWidgetClass = InElementWidgetClass;
    
    // 如果已有数据，则重新创建元素控件
    if (ArrayProperty && InnerProperty && TargetObject)
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
    SafeExecute([&]() {
        if (!ArrayProperty || !InnerProperty || !ContentVerticalBox || !TargetObject)
        {
            UE_LOG(LogTemp, Warning, TEXT("CreateElementWidgets失败: 缺少必要的属性或组件"));
            return;
        }
        
        // 清除现有控件
        ContentVerticalBox->ClearChildren();
        ElementUWidgets.Empty();
        
        // 获取数组地址
        void* ArrayPtr = ArrayProperty->ContainerPtrToValuePtr<void>(TargetObject);
        
        // 获取数组辅助类
        FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);
        
        // 遍历数组元素创建控件
        for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
        {
            CreateElementWidget(Index);
        }
    }, TEXT("创建数组元素控件失败"));
}

void URapidArrayPropertyWidget::CreateElementWidget(int32 ElementIndex)
{
    SafeExecute([&, ElementIndex]() {
        if (!ArrayProperty || !InnerProperty || !TargetObject || !ContentVerticalBox)
        {
            UE_LOG(LogTemp, Warning, TEXT("CreateElementWidget失败: 缺少必要的属性或组件"));
            return;
        }
        
        // 获取数组地址
        void* ArrayPtr = ArrayProperty->ContainerPtrToValuePtr<void>(TargetObject);
        
        // 获取数组辅助类
        FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);
        
        // 检查索引是否有效
        if (!ArrayHelper.IsValidIndex(ElementIndex))
        {
            UE_LOG(LogTemp, Warning, TEXT("CreateElementWidget失败: 索引无效 %d"), ElementIndex);
            return;
        }
        
        // 获取元素地址
        void* ElementValuePtr = ArrayHelper.GetRawPtr(ElementIndex);
        
        // 获取属性编辑器
        URapidPropertyEditor* PropertyEditor = Cast<URapidPropertyEditor>(GetTypedOuter<URapidPropertyEditor>());
        if (!PropertyEditor)
        {
            UE_LOG(LogTemp, Warning, TEXT("CreateElementWidget失败: 找不到父级属性编辑器"));
            return;
        }
        
        // 创建元素属性控件
        URapidPropertyWidget* ElementPropertyWidget = PropertyEditor->CreatePropertyWidgetForType(this, InnerProperty);
        
        if (!ElementPropertyWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("CreateElementWidget失败: 无法创建元素属性控件"));
            return;
        }
        
        // 设置元素名称为索引
        FName ElementPropertyName = *FString::Printf(TEXT("%s[%d]"), *PropertyName.ToString(), ElementIndex);
        
        // 初始化元素控件
        if (!ElementPropertyWidget->InitializePropertyWidget(TargetObject, InnerProperty, ElementPropertyName))
        {
            UE_LOG(LogTemp, Warning, TEXT("CreateElementWidget失败: 无法初始化元素属性控件"));
            return;
        }
        
        // 设置元素显示名称为索引
        ElementPropertyWidget->SetPropertyDisplayName(FText::AsNumber(ElementIndex));
        
        // 绑定值变化事件
        ElementPropertyWidget->OnPropertyValueChanged.AddDynamic(this, &URapidArrayPropertyWidget::HandleChildPropertyValueChanged);
        
        // 检查是否指定了元素小部件类
        if (!ElementWidgetClass)
        {
            UE_LOG(LogTemp, Warning, TEXT("CreateElementWidget失败: 未指定元素小部件类"));
            return;
        }
        
        // 创建自定义元素小部件
        URapidArrayElementWidget* ElementUWidget = CreateWidget<URapidArrayElementWidget>(this, ElementWidgetClass);
        if (!ElementUWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("CreateElementWidget失败: 无法创建元素UI小部件"));
            return;
        }
        
        // 初始化元素UI小部件
        ElementUWidget->InitializeElementWidget(TargetObject, InnerProperty, ElementValuePtr, ElementIndex);
        
        // 设置属性控件
        ElementUWidget->SetElementPropertyWidget(ElementPropertyWidget);
        
        // 绑定删除事件
        ElementUWidget->OnDeleteElementClicked.AddDynamic(this, &URapidArrayPropertyWidget::HandleElementDeleteClicked);
        
        // 添加到数组中
        ElementUWidgets.Add(ElementUWidget);
        
        // 添加到内容垂直框
        ContentVerticalBox->AddChild(ElementUWidget);
    }, TEXT("创建数组元素UI失败"));
}

void URapidArrayPropertyWidget::UpdateElementWidgets()
{
    SafeExecute([&]() {
        if (!ArrayProperty || !InnerProperty || !TargetObject)
        {
            UE_LOG(LogTemp, Warning, TEXT("UpdateElementWidgets失败: 缺少必要的属性"));
            return;
        }
        
        // 获取数组地址
        void* ArrayPtr = ArrayProperty->ContainerPtrToValuePtr<void>(TargetObject);
        
        // 检查数组元素数量是否发生变化
        FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);
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
    }, TEXT("更新数组元素控件失败"));
}

void URapidArrayPropertyWidget::HandleAddElementClicked()
{
    SafeExecute([&]() {
        if (!ArrayProperty || !InnerProperty || !TargetObject)
        {
            UE_LOG(LogTemp, Warning, TEXT("添加数组元素失败: 缺少必要的属性"));
            return;
        }
        
        // 获取数组地址
        void* ArrayPtr = ArrayProperty->ContainerPtrToValuePtr<void>(TargetObject);
        
        // 获取数组辅助类
        FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);
        
        // 添加新元素
        const int32 NewIndex = ArrayHelper.AddValue();
        
        // 创建新元素的控件
        CreateElementWidget(NewIndex);
        
        // 通知修改
        NotifyPropertyValueChanged();
    }, TEXT("添加数组元素失败"));
}

void URapidArrayPropertyWidget::HandleElementDeleteClicked(int32 ElementIndex)
{
    SafeExecute([&, ElementIndex]() {
        if (!ArrayProperty || !InnerProperty || !TargetObject)
        {
            UE_LOG(LogTemp, Warning, TEXT("删除数组元素失败: 缺少必要的属性"));
            return;
        }
        
        // 获取数组地址
        void* ArrayPtr = ArrayProperty->ContainerPtrToValuePtr<void>(TargetObject);
        
        // 获取数组辅助类
        FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayPtr);
        
        // 检查索引是否有效
        if (!ArrayHelper.IsValidIndex(ElementIndex))
        {
            UE_LOG(LogTemp, Warning, TEXT("删除数组元素失败: 索引无效 %d"), ElementIndex);
            return;
        }
        
        // 删除元素
        ArrayHelper.RemoveValues(ElementIndex, 1);
        
        // 重新创建所有元素控件
        CreateElementWidgets();
        
        // 通知修改
        NotifyPropertyValueChanged();
    }, TEXT("删除数组元素失败"));
}

void URapidArrayPropertyWidget::HandleChildPropertyValueChanged(UObject* Object, FName InPropertyName, URapidPropertyWidget* PropertyWidget)
{
    // 传递子元素变化事件
    NotifyPropertyValueChanged();
}