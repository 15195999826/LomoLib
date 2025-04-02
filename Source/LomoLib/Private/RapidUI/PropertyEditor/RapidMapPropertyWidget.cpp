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

bool URapidMapPropertyWidget::InitializePropertyWidget(UObject* InObject, FProperty* InProperty, const FName& InPropertyName)
{
    // 调用父类的初始化方法
    if (!Super::InitializePropertyWidget(InObject, InProperty, InPropertyName))
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidMapPropertyWidget初始化失败: 父类初始化返回false"));
        return false;
    }

    // 参数校验
    if (!InObject || !InProperty)
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidMapPropertyWidget初始化失败: 对象或属性为空"));
        return false;
    }

    // 保存映射属性引用
    MapProperty = CastField<FMapProperty>(InProperty);
    if (!MapProperty)
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidMapPropertyWidget初始化失败: 属性类型不是Map"));
        return false;
    }

    // 获取键和值属性
    KeyProperty = MapProperty->KeyProp;
    ValueProperty = MapProperty->ValueProp;

    if (!KeyProperty || !ValueProperty)
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidMapPropertyWidget初始化失败: 键或值属性为空"));
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
        AddElementButton->OnClicked.AddDynamic(this, &URapidMapPropertyWidget::HandleAddElementClicked);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RapidMapPropertyWidget初始化失败: 添加元素按钮为空"));
        return false;
    }

    // 创建元素控件
    CreatePairWidgets();
    
    return true;
}

TSubclassOf<URapidMapElementWidget> URapidMapPropertyWidget::GetElementWidgetClass() const
{
    return ElementWidgetClass;
}

void URapidMapPropertyWidget::SetElementWidgetClass(TSubclassOf<URapidMapElementWidget> InElementWidgetClass)
{
    ElementWidgetClass = InElementWidgetClass;
    
    // 如果已有数据，则重新创建元素控件
    if (MapProperty && KeyProperty && ValueProperty && TargetObject)
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
    SafeExecute([&]() {
        if (!MapProperty || !KeyProperty || !ValueProperty || !ContentVerticalBox || !TargetObject)
        {
            UE_LOG(LogTemp, Warning, TEXT("CreatePairWidgets失败: 缺少必要的属性或组件"));
            return;
        }

        // 清除现有控件
        ContentVerticalBox->ClearChildren();
        ElementUWidgets.Empty();

        // 获取映射地址
        void* MapPtr = MapProperty->ContainerPtrToValuePtr<void>(TargetObject);
        
        // 获取映射辅助类
        FScriptMapHelper MapHelper(MapProperty, MapPtr);

        // 遍历映射元素创建控件
        for (int32 Index = 0; Index < MapHelper.Num(); ++Index)
        {
            CreatePairWidget(Index);
        }
    }, TEXT("创建映射元素控件失败"));
}

void URapidMapPropertyWidget::CreatePairWidget(int32 PairIndex)
{
    SafeExecute([&, PairIndex]() {
        if (!MapProperty || !KeyProperty || !ValueProperty || !TargetObject || !ContentVerticalBox)
        {
            UE_LOG(LogTemp, Warning, TEXT("CreatePairWidget失败: 缺少必要的属性或组件"));
            return;
        }

        // 获取映射地址
        void* MapPtr = MapProperty->ContainerPtrToValuePtr<void>(TargetObject);
        
        // 获取映射辅助类
        FScriptMapHelper MapHelper(MapProperty, MapPtr);

        // 检查索引是否有效
        if (PairIndex < 0 || PairIndex >= MapHelper.Num())
        {
            UE_LOG(LogTemp, Warning, TEXT("CreatePairWidget失败: 索引无效 %d"), PairIndex);
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
            UE_LOG(LogTemp, Warning, TEXT("CreatePairWidget失败: 无法找到有效的稀疏索引"));
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
            UE_LOG(LogTemp, Warning, TEXT("CreatePairWidget失败: 找不到父级属性编辑器"));
            return;
        }

        // 创建键控件
        URapidPropertyWidget* KeyWidget = PropertyEditor->CreatePropertyWidgetForType(this, KeyProperty);
        if (!KeyWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("CreatePairWidget失败: 无法创建键属性控件"));
            return;
        }

        // 设置键控件属性名称
        FName KeyPropertyName = *FString::Printf(TEXT("%s[%d].Key"), *PropertyName.ToString(), PairIndex);
        
        // 初始化键控件
        if (!KeyWidget->InitializePropertyWidget(TargetObject, KeyProperty, KeyPropertyName))
        {
            UE_LOG(LogTemp, Warning, TEXT("CreatePairWidget失败: 无法初始化键属性控件"));
            return;
        }
        
        // 设置键控件显示名称
        KeyWidget->SetPropertyDisplayName(FText::FromString(TEXT("键")));
        
        // 绑定值变化事件
        KeyWidget->OnPropertyValueChanged.AddDynamic(this, &URapidMapPropertyWidget::HandleChildPropertyValueChanged);

        // 创建值控件
        URapidPropertyWidget* ValueWidget = PropertyEditor->CreatePropertyWidgetForType(this, ValueProperty);
        if (!ValueWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("CreatePairWidget失败: 无法创建值属性控件"));
            return;
        }

        // 设置值控件属性名称
        FName ValuePropertyName = *FString::Printf(TEXT("%s[%d].Value"), *PropertyName.ToString(), PairIndex);
        
        // 初始化值控件
        if (!ValueWidget->InitializePropertyWidget(TargetObject, ValueProperty, ValuePropertyName))
        {
            UE_LOG(LogTemp, Warning, TEXT("CreatePairWidget失败: 无法初始化值属性控件"));
            return;
        }
        
        // 设置值控件显示名称
        ValueWidget->SetPropertyDisplayName(FText::FromString(TEXT("值")));
        
        // 绑定值变化事件
        ValueWidget->OnPropertyValueChanged.AddDynamic(this, &URapidMapPropertyWidget::HandleChildPropertyValueChanged);

        // 检查是否指定了元素小部件类
        if (!ElementWidgetClass)
        {
            UE_LOG(LogTemp, Warning, TEXT("CreatePairWidget失败: 未指定元素小部件类"));
            return;
        }
        
        // 创建自定义元素小部件
        URapidMapElementWidget* ElementUWidget = CreateWidget<URapidMapElementWidget>(this, ElementWidgetClass);
        if (!ElementUWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("CreatePairWidget失败: 无法创建元素UI小部件"));
            return;
        }
        
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
    }, TEXT("创建映射元素UI失败"));
}

void URapidMapPropertyWidget::UpdatePairWidgets()
{
    SafeExecute([&]() {
        if (!MapProperty || !KeyProperty || !ValueProperty || !TargetObject)
        {
            UE_LOG(LogTemp, Warning, TEXT("UpdatePairWidgets失败: 缺少必要的属性"));
            return;
        }
        
        // 获取映射地址
        void* MapPtr = MapProperty->ContainerPtrToValuePtr<void>(TargetObject);
        
        // 检查映射元素数量是否发生变化
        FScriptMapHelper MapHelper(MapProperty, MapPtr);
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
    }, TEXT("更新映射元素控件失败"));
}

void URapidMapPropertyWidget::HandleAddElementClicked()
{
    SafeExecute([&]() {
        if (!MapProperty || !KeyProperty || !ValueProperty || !TargetObject)
        {
            UE_LOG(LogTemp, Warning, TEXT("添加映射元素失败: 缺少必要的属性"));
            return;
        }
        
        // 获取映射地址
        void* MapPtr = MapProperty->ContainerPtrToValuePtr<void>(TargetObject);
        
        // 获取映射辅助类
        FScriptMapHelper MapHelper(MapProperty, MapPtr);
        
        // 创建一个默认键
        void* DefaultKeyPtr = FMemory::Malloc(KeyProperty->GetSize(), KeyProperty->GetMinAlignment());
        KeyProperty->InitializeValue(DefaultKeyPtr);
        
        // 创建一个默认值
        void* DefaultValuePtr = FMemory::Malloc(ValueProperty->GetSize(), ValueProperty->GetMinAlignment());
        ValueProperty->InitializeValue(DefaultValuePtr);
        
        // 添加新元素
        int32 NewIndex = MapHelper.AddDefaultValue_Invalid_NeedsRehash();
        uint8* NewPairPtr = MapHelper.GetPairPtr(NewIndex);
        
        // 复制键和值
        KeyProperty->CopyCompleteValue(NewPairPtr, DefaultKeyPtr);
        ValueProperty->CopyCompleteValue(NewPairPtr + KeyProperty->GetSize(), DefaultValuePtr);
        
        // 重新创建所有元素控件
        MapHelper.Rehash();
        CreatePairWidgets();
        
        // 通知修改
        NotifyPropertyValueChanged();
        
        // 释放临时内存
        KeyProperty->DestroyValue(DefaultKeyPtr);
        ValueProperty->DestroyValue(DefaultValuePtr);
        FMemory::Free(DefaultKeyPtr);
        FMemory::Free(DefaultValuePtr);
    }, TEXT("添加映射元素失败"));
}

void URapidMapPropertyWidget::HandleElementDeleteClicked(int32 ElementIndex)
{
    SafeExecute([&, ElementIndex]() {
        if (!MapProperty || !KeyProperty || !ValueProperty || !TargetObject)
        {
            UE_LOG(LogTemp, Warning, TEXT("删除映射元素失败: 缺少必要的属性"));
            return;
        }
        
        // 获取映射地址
        void* MapPtr = MapProperty->ContainerPtrToValuePtr<void>(TargetObject);
        
        // 获取映射辅助类
        FScriptMapHelper MapHelper(MapProperty, MapPtr);
        
        // 检查是否有效的索引
        if (ElementIndex < 0 || ElementIndex >= ElementUWidgets.Num())
        {
            UE_LOG(LogTemp, Warning, TEXT("删除映射元素失败: 无效的索引 %d"), ElementIndex);
            return;
        }
        
        // 找到对应的稀疏索引
        int32 RealIndex = 0;
        int32 SparseIndex = -1;
        for (int32 i = 0; i < MapHelper.GetMaxIndex(); ++i)
        {
            if (MapHelper.IsValidIndex(i))
            {
                if (RealIndex == ElementIndex)
                {
                    SparseIndex = i;
                    break;
                }
                RealIndex++;
            }
        }
        
        if (SparseIndex != -1)
        {
            // 删除元素
            MapHelper.RemoveAt(SparseIndex);
            
            // 重新创建所有元素控件
            CreatePairWidgets();
            
            // 通知修改
            NotifyPropertyValueChanged();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("删除映射元素失败: 无法找到元素的稀疏索引"));
        }
    }, TEXT("删除映射元素失败"));
}

void URapidMapPropertyWidget::HandleChildPropertyValueChanged(UObject* Object, FName InPropertyName, URapidPropertyWidget* PropertyWidget)
{
    // 传递子属性变化事件
    NotifyPropertyValueChanged();
}