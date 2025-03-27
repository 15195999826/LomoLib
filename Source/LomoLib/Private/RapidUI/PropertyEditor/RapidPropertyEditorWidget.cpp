// Fill out your copyright notice in the Description page of Project Settings.

#include "RapidUI/PropertyEditor/RapidPropertyEditorWidget.h"
#include "RapidUI/PropertyEditor/RapidPropertyEditor.h"
#include "RapidUI/PropertyEditor/LomoLibPropertyEditorTestObject.h"
#include "Components/Button.h"
#include "Engine/Engine.h"

void URapidPropertyEditorWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 创建测试对象
    TestObject = NewObject<ULomoLibPropertyEditorTestObject>(this);
    
    // 绑定按钮点击事件
    if (RefreshButton)
    {
        RefreshButton->OnClicked.AddDynamic(this, &URapidPropertyEditorWidget::HandleRefreshButtonClicked);
    }
    
    if (ResetButton)
    {
        ResetButton->OnClicked.AddDynamic(this, &URapidPropertyEditorWidget::HandleResetButtonClicked);
    }
    
    // 设置属性编辑器
    if (PropertyEditor && TestObject)
    {
        // PropertyEditor->OnPropertyChanged.AddDynamic(this, &URapidPropertyEditorWidget::HandlePropertyChanged);
        PropertyEditor->SetObject(TestObject);
    }
}

void URapidPropertyEditorWidget::NativeDestruct()
{
    // 解绑按钮点击事件
    if (RefreshButton)
    {
        RefreshButton->OnClicked.RemoveAll(this);
    }
    
    if (ResetButton)
    {
        ResetButton->OnClicked.RemoveAll(this);
    }
    
    // 清空属性编辑器
    if (PropertyEditor)
    {
        // PropertyEditor->OnPropertyChanged.RemoveAll(this);
        PropertyEditor->ClearObject();
    }
    
    // 释放测试对象
    TestObject = nullptr;
    
    Super::NativeDestruct();
}

void URapidPropertyEditorWidget::HandleRefreshButtonClicked()
{
    if (PropertyEditor)
    {
        PropertyEditor->Refresh();
    }
}

void URapidPropertyEditorWidget::HandleResetButtonClicked()
{
    // 重新创建测试对象
    TestObject = NewObject<ULomoLibPropertyEditorTestObject>(this);
    
    // 重新设置属性编辑器
    if (PropertyEditor && TestObject)
    {
        PropertyEditor->SetObject(TestObject);
    }
}

// void URapidPropertyEditorWidget::HandlePropertyChanged(UObject* Object, FName PropertyName, const FProperty* Property)
// {
//     // 显示属性改变信息
//     if (GEngine && Object && Property)
//     {
//         FString DisplayValue;
//         void* ValuePtr = const_cast<FProperty*>(Property)->ContainerPtrToValuePtr<void>(Object);
//         // Property->ExportTextItem(DisplayValue, ValuePtr, nullptr, Object, PPF_None);
//         
//         FString Message = FString::Printf(TEXT("属性已修改: %s = %s"), *PropertyName.ToString(), *DisplayValue);
//         GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, Message);
//     }
// } 