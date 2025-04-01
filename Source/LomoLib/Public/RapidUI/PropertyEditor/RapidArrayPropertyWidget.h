#pragma once

#include "CoreMinimal.h"
#include "RapidUI/PropertyEditor/RapidPropertyWidget.h"
#include "RapidArrayPropertyWidget.generated.h"

class UTextBlock;
class UExpandableArea;
class UVerticalBox;
class UButton;
class UScrollBox;
class URapidArrayElementWidget;

/**
 * 数组类型的属性控件，支持TArray
 */
UCLASS(BlueprintType, Blueprintable)
class LOMOLIB_API URapidArrayPropertyWidget : public URapidPropertyWidget
{
    GENERATED_BODY()

public:
    URapidArrayPropertyWidget(const FObjectInitializer& ObjectInitializer);
    
    // 重写初始化方法
    virtual bool InitializePropertyWidget(UObject* InObject, FProperty* InProperty, const FName& InPropertyName) override;
    
    // 重写更新值方法
    virtual void UpdateValue_Implementation() override;

    /** 获取元素小部件类 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    TSubclassOf<URapidArrayElementWidget> GetElementWidgetClass() const;
    
    /** 设置元素小部件类 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    void SetElementWidgetClass(TSubclassOf<URapidArrayElementWidget> InElementWidgetClass);
    
protected:
    // 标题文本
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* TitleText;
    
    // 可展开区域
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UExpandableArea* ExpandableArea;
    
    // 内容垂直框
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UVerticalBox* ContentVerticalBox;
    
    // 添加元素按钮
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* AddElementButton;
    
    // 元素小部件类 - 用于创建数组元素界面 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property Widget")
    TSubclassOf<URapidArrayElementWidget> ElementWidgetClass;
    
    // 处理添加元素按钮点击事件
    UFUNCTION()
    void HandleAddElementClicked();
    
    // 处理元素删除按钮点击事件
    UFUNCTION()
    void HandleElementDeleteClicked(int32 ElementIndex);
    
    // 处理子属性值改变
    UFUNCTION()
    void HandleChildPropertyValueChanged(UObject* Object, FName InPropertyName, URapidPropertyWidget* PropertyWidget);
    
private:
    // 创建数组中的所有元素控件
    void CreateElementWidgets();
    
    // 创建单个数组元素控件
    void CreateElementWidget(int32 ElementIndex);
    
    // 更新所有元素的显示
    void UpdateElementWidgets();
    
    // 内部保存的数组属性
    FArrayProperty* ArrayProperty;
    
    // 内部数组元素属性
    FProperty* InnerProperty;
    
    // 生成的元素小部件
    UPROPERTY()
    TArray<URapidArrayElementWidget*> ElementUWidgets;
};