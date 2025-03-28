#pragma once

#include "CoreMinimal.h"
#include "RapidUI/PropertyEditor/RapidPropertyWidget.h"
#include "RapidMapPropertyWidget.generated.h"

class UTextBlock;
class UExpandableArea;
class UVerticalBox;
class UButton;
class UScrollBox;
class URapidMapElementWidget;

/**
 * 映射类型的属性控件，支持TMap
 */
UCLASS(BlueprintType, Blueprintable)
class LOMOLIB_API URapidMapPropertyWidget : public URapidPropertyWidget
{
    GENERATED_BODY()

public:
    URapidMapPropertyWidget(const FObjectInitializer& ObjectInitializer);
    
    // 重写初始化方法
    virtual void InitializePropertyWidget(UObject* InObject, FProperty* InProperty, void* InValuePtr) override;
    
    // 重写更新值方法
    virtual void UpdateValue_Implementation() override;
    
    /** 获取元素小部件类 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    TSubclassOf<URapidMapElementWidget> GetElementWidgetClass() const;
    
    /** 设置元素小部件类 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    void SetElementWidgetClass(TSubclassOf<URapidMapElementWidget> InElementWidgetClass);
    
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
    
    // 元素小部件类 - 用于创建映射元素界面 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property Widget")
    TSubclassOf<URapidMapElementWidget> ElementWidgetClass;
    
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
    // 创建映射中的所有元素控件
    void CreatePairWidgets();
    
    // 创建单个映射元素控件
    void CreatePairWidget(int32 PairIndex);
    
    // 更新所有元素的显示
    void UpdatePairWidgets();
    
    // 内部保存的映射属性
    FMapProperty* MapProperty;
    
    // 内部键属性
    FProperty* KeyProperty;
    
    // 内部值属性
    FProperty* ValueProperty;
    
    // 生成的元素小部件
    UPROPERTY()
    TArray<URapidMapElementWidget*> ElementUWidgets;
};