// Fill out your copyright notice in the Description page of Project Settings.
/**
 * RapidPropertyEditor自定义属性控件使用指南
 * 
 * 本文档描述了如何使用RapidPropertyEditor的自定义属性控件功能，
 * 以便通过蓝图自定义各种属性控件的样式。
 * 
 * 一、基本架构
 * -------------
 * 1. URapidPropertyWidget - 基础属性控件类，所有具体的属性控件都继承自此类
 * 2. 具体类型的属性控件:
 *    - URapidFloatPropertyWidget - 浮点数属性控件
 *    - URapidIntPropertyWidget - 整数属性控件
 *    - URapidBoolPropertyWidget - 布尔属性控件
 *    - URapidStringPropertyWidget - 字符串属性控件 (支持FString, FName, FText)
 *    - URapidStructPropertyWidget - 结构体属性控件
 * 
 * 二、如何创建自定义属性控件蓝图
 * -----------------------------
 * 1. 为每种属性类型创建对应的蓝图类:
 *    a. 在内容浏览器中右键点击，选择"蓝图类"
 *    b. 在"所有类"中搜索对应的属性控件类(如URapidFloatPropertyWidget)
 *    c. 创建并命名蓝图(如BP_CustomFloatProperty)
 * 
 * 2. 设计蓝图UI:
 *    a. 打开蓝图编辑器，进入"设计器"标签
 *    b. 每种属性控件都需要包含特定的命名组件，例如:
 *       - URapidFloatPropertyWidget需要包含:
 *         > PropertyNameText (UTextBlock) - 显示属性名称
 *         > ValueSpinBox (USpinBox) - 编辑浮点值
 *       - URapidBoolPropertyWidget需要包含:
 *         > PropertyNameText (UTextBlock) - 显示属性名称
 *         > ValueCheckBox (UCheckBox) - 编辑布尔值
 * 
 * 3. 自定义样式:
 *    可以自由设计UI布局和样式，只要保持必要的命名组件存在即可。
 *    可以添加背景、边框、图标等额外元素丰富UI表现。
 * 
 * 三、配置RapidPropertyEditor使用自定义控件
 * --------------------------------------
 * 1. 创建RapidPropertyEditor蓝图:
 *    a. 创建一个继承自URapidPropertyEditor的蓝图
 *    b. 在蓝图详情面板中设置各种属性控件类:
 *       - Float Property Widget Class: 选择你创建的BP_CustomFloatProperty
 *       - Int Property Widget Class: 选择你创建的BP_CustomIntProperty
 *       - 以此类推设置其他类型
 * 
 * 2. 在UI中使用:
 *    将你创建的RapidPropertyEditor蓝图放入你的UI中，
 *    然后调用SetObject函数设置要编辑的对象。
 * 
 * 四、示例 - 浮点数属性控件蓝图设计
 * --------------------------------
 * 1. 组件结构:
 *    - Root Canvas
 *      |- HorizontalBox
 *         |- PropertyNameText (UTextBlock) [绑定]
 *         |- ValueSpinBox (USpinBox) [绑定]
 * 
 * 2. 样式设置:
 *    - 为HorizontalBox添加背景和内边距
 *    - 自定义PropertyNameText的字体、颜色、大小
 *    - 自定义ValueSpinBox的样式、箭头颜色等
 * 
 * 五、注意事项
 * -----------
 * 1. 必须保持特定命名的组件，否则属性控件将无法正常工作
 * 2. 可以添加任意额外组件来丰富UI，不会影响功能
 * 3. 所有属性控件都会自动处理属性的读取和修改
 */ 

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RapidPropertyEditor.generated.h"

class USpinBox;
class UScrollBox;
class UVerticalBox;
class UButton;
class URapidPropertyWidget;

/**
 * 运行时属性编辑器，类似于编辑器中的DetailsView
 * 允许在游戏中编辑UObject属性
 */
UCLASS()
class LOMOLIB_API URapidPropertyEditor : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/**
	 * 设置要显示和编辑的对象
	 * @param InObject 要编辑的对象
	 * @return 是否成功设置对象
	 */
	UFUNCTION(BlueprintCallable, Category = "Property Editor")
	bool SetObject(UObject* InObject);

	/**
	 * 获取当前正在编辑的对象
	 * @return 当前编辑的对象
	 */
	UFUNCTION(BlueprintCallable, Category = "Property Editor")
	UObject* GetObject() const;

	/**
	 * 刷新属性编辑器
	 */
	UFUNCTION(BlueprintCallable, Category = "Property Editor")
	void Refresh();

	/**
	 * 清空当前显示的对象
	 */
	UFUNCTION(BlueprintCallable, Category = "Property Editor")
	void ClearObject();

	TObjectPtr<URapidPropertyWidget> CreatePropertyWidgetForType(UUserWidget* InOuter, const FProperty* InProperty);
	/**
	 * 属性改变时触发的委托
	 */
	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPropertyChangedDelegate, UObject*, Object, FName, PropertyName, FProperty*, Property);
	
	/**
	 * 当任何属性改变时触发
	 */
	// UPROPERTY(BlueprintAssignable, Category = "Property Editor")
	// FOnPropertyChangedDelegate OnPropertyChanged;

protected:
	/** 内容容器 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScrollBox* ContentScrollBox;

	// -------- 属性控件类引用 Start ----------
	/** 浮点数属性控件类 */
	UPROPERTY(EditDefaultsOnly, Category = "Property Editor|Widget Classes")
	TSubclassOf<URapidPropertyWidget> FloatPropertyWidgetClass;
	
	/** 整数属性控件类 */
	UPROPERTY(EditDefaultsOnly, Category = "Property Editor|Widget Classes")
	TSubclassOf<URapidPropertyWidget> IntPropertyWidgetClass;
	
	/** 布尔属性控件类 */
	UPROPERTY(EditDefaultsOnly, Category = "Property Editor|Widget Classes")
	TSubclassOf<URapidPropertyWidget> BoolPropertyWidgetClass;
	
	/** 字符串属性控件类 */
	UPROPERTY(EditDefaultsOnly, Category = "Property Editor|Widget Classes")
	TSubclassOf<URapidPropertyWidget> StringPropertyWidgetClass;
	
	/** 结构体属性控件类 */
	UPROPERTY(EditDefaultsOnly, Category = "Property Editor|Widget Classes")
	TSubclassOf<URapidPropertyWidget> StructPropertyWidgetClass;
	
	/** 数组属性控件类 */
	UPROPERTY(EditDefaultsOnly, Category = "Property Editor|Widget Classes")
	TSubclassOf<URapidPropertyWidget> ArrayPropertyWidgetClass;
	
	/** 映射属性控件类 */
	UPROPERTY(EditDefaultsOnly, Category = "Property Editor|Widget Classes")
	TSubclassOf<URapidPropertyWidget> MapPropertyWidgetClass;
	// -------- 属性控件类引用 End -------------

	/** 创建并渲染属性面板内容 */
	void RenderProperties();

	/** 处理属性值改变 */
	UFUNCTION()
	void HandlePropertyValueChanged(UObject* Object, FName PropertyName, URapidPropertyWidget* PropertyWidget);

private:
	/** 当前编辑的对象 */
	UPROPERTY(Transient)
	UObject* TargetObject;

	/** 已创建的属性控件 */
	UPROPERTY()
	TArray<URapidPropertyWidget*> PropertyWidgets;

	/** 获取属性显示名称 */
	FString GetPropertyDisplayName(FProperty* Property) const;

	/** 通知属性改变 */
	void NotifyPropertyChanged(FName PropertyName, FProperty* Property = nullptr);
};