#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RapidPropertyWidget.h"
#include "RapidMapElementWidget.generated.h"

class UButton;
class UTextBlock;
class UBorder;
/**
 * 映射元素小部件类，用于在蓝图中设计映射元素的UI
 */
UCLASS(BlueprintType, Blueprintable)
class LOMOLIB_API URapidMapElementWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    URapidMapElementWidget(const FObjectInitializer& ObjectInitializer);
    
    /** 初始化映射元素小部件 */
    virtual void InitializeElementWidget(UObject* InObject, FProperty* InKeyProperty, FProperty* InValueProperty, 
                                         void* InKeyValuePtr, void* InValueValuePtr, int32 InPairIndex);
    
    /** 更新元素值 */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Property Widget")
    void UpdateValue();
    
    /** 获取键属性控件 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    URapidPropertyWidget* GetKeyPropertyWidget() const;
    
    /** 设置键属性控件 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    void SetKeyPropertyWidget(URapidPropertyWidget* InPropertyWidget);
    
    /** 获取值属性控件 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    URapidPropertyWidget* GetValuePropertyWidget() const;
    
    /** 设置值属性控件 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    void SetValuePropertyWidget(URapidPropertyWidget* InPropertyWidget);
    
    /** 获取元素索引 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    int32 GetPairIndex() const;
    
    /** 删除元素事件 */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeletePairClickedDelegate, int32, PairIndex);
    
    /** 当删除按钮被点击时触发 */
    UPROPERTY(BlueprintAssignable, Category = "Property Widget")
    FOnDeletePairClickedDelegate OnDeletePairClicked;
    
protected:
    /** 元素索引文本组件 - 可在蓝图中绑定 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> IndexText;
    
    /** 键属性容器 - 可在蓝图中绑定 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UBorder> KeyContainer;
    
    /** 值属性容器 - 可在蓝图中绑定 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UBorder> ValueContainer;
    
    /** 删除按钮 - 可在蓝图中绑定 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UButton> DeleteButton;
    
    /** 处理删除按钮点击事件 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    void HandleDeleteButtonClicked();
    
    /** 当前元素的索引 */
    UPROPERTY(BlueprintReadOnly, Category = "Property Widget")
    int32 PairIndex;
    
    /** 键属性控件 */
    UPROPERTY(BlueprintReadOnly, Category = "Property Widget")
    URapidPropertyWidget* KeyPropertyWidget;
    
    /** 值属性控件 */
    UPROPERTY(BlueprintReadOnly, Category = "Property Widget")
    URapidPropertyWidget* ValuePropertyWidget;
    
    /** 初始化组件后设置控件 */
    virtual void NativeConstruct() override;
}; 