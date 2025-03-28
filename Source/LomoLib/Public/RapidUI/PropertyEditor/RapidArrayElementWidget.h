#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RapidPropertyWidget.h"
#include "RapidArrayElementWidget.generated.h"

class UTextBlock;
class UBorder;
class UButton;
/**
 * 数组元素小部件类，用于在蓝图中设计数组元素的UI
 */
UCLASS(BlueprintType, Blueprintable)
class LOMOLIB_API URapidArrayElementWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    URapidArrayElementWidget(const FObjectInitializer& ObjectInitializer);
    
    /** 初始化数组元素小部件 */
    virtual void InitializeElementWidget(UObject* InObject, FProperty* InElementProperty, void* InElementValuePtr, int32 InElementIndex);
    
    /** 更新元素值 */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Property Widget")
    void UpdateValue();
    
    /** 获取元素属性控件 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    URapidPropertyWidget* GetElementPropertyWidget() const;
    
    /** 设置元素属性控件 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    void SetElementPropertyWidget(URapidPropertyWidget* InPropertyWidget);
    
    /** 获取元素索引 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    int32 GetElementIndex() const;
    
    /** 删除元素事件 */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeleteElementClickedDelegate, int32, ElementIndex);
    
    /** 当删除按钮被点击时触发 */
    UPROPERTY(BlueprintAssignable, Category = "Property Widget")
    FOnDeleteElementClickedDelegate OnDeleteElementClicked;
    
protected:
    /** 元素索引文本组件 - 可在蓝图中绑定 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> IndexText;
    
    /** 元素属性容器 - 可在蓝图中绑定 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UBorder> ElementContainer;
    
    /** 删除按钮 - 可在蓝图中绑定 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UButton> DeleteButton;
    
    /** 处理删除按钮点击事件 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    void HandleDeleteButtonClicked();
    
    /** 当前元素的索引 */
    UPROPERTY(BlueprintReadOnly, Category = "Property Widget")
    int32 ElementIndex;
    
    /** 元素属性控件 */
    UPROPERTY(BlueprintReadOnly, Category = "Property Widget")
    URapidPropertyWidget* ElementPropertyWidget;
    
    /** 初始化组件后设置控件 */
    virtual void NativeConstruct() override;
}; 