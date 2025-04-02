// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RapidPropertyWidget.generated.h"

/**
 * 基础属性控件类，所有具体的属性控件都应该继承这个类
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class LOMOLIB_API URapidPropertyWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    URapidPropertyWidget(const FObjectInitializer& ObjectInitializer);

    /** 获取当前属性名 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    FName GetPropertyName() const;

    /** 设置属性名 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    void SetPropertyName(FName InPropertyName);

    /** 获取属性显示名称 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    FText GetPropertyDisplayName() const;

    /** 设置属性显示名称 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    void SetPropertyDisplayName(const FText& InDisplayName);

    /** 初始化属性控件 */
    virtual bool InitializePropertyWidget(UObject* InObject, FProperty* InProperty, const FName& InPropertyName);

    /** 更新属性值（当值改变时） */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Property Widget")
    void UpdateValue();

    /** 值改变时触发的委托 */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPropertyValueChangedDelegate, UObject*, Object, FName, PropertyName, URapidPropertyWidget*, PropertyWidget);

    /** 当控件中的值被用户改变时触发 */
    UPROPERTY(BlueprintAssignable, Category = "Property Widget")
    FOnPropertyValueChangedDelegate OnPropertyValueChanged;

protected:
    /** 当前编辑的对象 */
    UPROPERTY(BlueprintReadOnly, Category = "Property Widget")
    UObject* TargetObject;

    /** 属性指针 */
    FProperty* Property;

    /** 属性名称 */
    UPROPERTY(BlueprintReadOnly, Category = "Property Widget")
    FName PropertyName;

    /** 属性显示名称 */
    UPROPERTY(BlueprintReadOnly, Category = "Property Widget")
    FText PropertyDisplayName;

    /** 获取属性的元数据值 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    FString GetPropertyMetaData(const FName& MetaDataKey) const;

    /** 通知属性值已经改变 */
    UFUNCTION(BlueprintCallable, Category = "Property Widget")
    void NotifyPropertyValueChanged();
    
    /** 安全执行方法的辅助函数，统一处理异常 */
    template<typename F>
    void SafeExecute(F&& Operation, const TCHAR* ErrorMessage)
    {
        try
        {
            Operation();
        }
        catch (const std::exception& Ex)
        {
            UE_LOG(LogTemp, Error, TEXT("%s - 异常: %s"), ErrorMessage, UTF8_TO_TCHAR(Ex.what()));
        }
        catch (...)
        {
            UE_LOG(LogTemp, Error, TEXT("%s - 未知异常"), ErrorMessage);
        }
    }
    
    /** 安全执行方法的辅助函数，带返回值，统一处理异常 */
    template<typename F, typename RetType = decltype(std::declval<F>()())>
    RetType SafeExecuteWithRet(F&& Operation, const TCHAR* ErrorMessage)
    {
        try
        {
            return Operation();
        }
        catch (const std::exception& Ex)
        {
            UE_LOG(LogTemp, Error, TEXT("%s - 异常: %s"), ErrorMessage, UTF8_TO_TCHAR(Ex.what()));
            return RetType();
        }
        catch (...)
        {
            UE_LOG(LogTemp, Error, TEXT("%s - 未知异常"), ErrorMessage);
            return RetType();
        }
    }
};