// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RapidPropertyEditor.generated.h"

// 前向声明
class UScrollBox;
class UVerticalBox;
class UButton;
class UComboBoxString;
class UUserWidget;

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

	/**
	 * 属性改变时触发的委托
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPropertyChangedDelegate, UObject*, Object, FName, PropertyName, const FProperty*, Property);
	
	/**
	 * 当任何属性改变时触发
	 */
	UPROPERTY(BlueprintAssignable, Category = "Property Editor")
	FOnPropertyChangedDelegate OnPropertyChanged;

	/**
	 * 设置控件模板类
	 * @param InIntSpinBoxClass Int类型SpinBox的模板类
	 * @param InFloatSpinBoxClass Float类型SpinBox的模板类
	 * @param InTextBoxClass 文本框的模板类
	 * @param InCheckBoxClass 复选框的模板类
	 * @param InComboBoxClass 下拉框的模板类（枚举类型使用）
	 */
	UFUNCTION(BlueprintCallable, Category = "Property Editor")
	void SetWidgetTemplates(TSubclassOf<UUserWidget> InLabelClass, 
						   TSubclassOf<USpinBox> InFloatSpinBoxClass,
						   TSubclassOf<USpinBox> InIntSpinBoxClass,
						   TSubclassOf<UEditableTextBox> InTextBoxClass,
						   TSubclassOf<UCheckBox> InCheckBoxClass,
						   TSubclassOf<UComboBoxString> InComboBoxClass);

protected:
	/** 内容容器 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UScrollBox* ContentScrollBox;

	/** 创建并渲染属性面板内容 */
	void RenderProperties();

	/** 创建单个属性的控件 */
	UWidget* CreatePropertyWidget(FProperty* Property, void* PropertyValue);
	
	/** 创建基本类型属性控件 (float, int, FString等) */
	UWidget* CreateBasicPropertyWidget(FProperty* Property, void* PropertyValue);

	/** 创建枚举类型属性控件 */
	UWidget* CreateEnumPropertyWidget(FEnumProperty* EnumProperty, void* PropertyValue);

	/** 创建枚举类型属性控件(旧版本UE兼容) */
	UWidget* CreateBytePropertyWidget(FByteProperty* ByteProperty, void* PropertyValue);

	/** 创建TArray类型属性控件 */
	UWidget* CreateArrayPropertyWidget(FArrayProperty* ArrayProperty, void* PropertyValue);

	/** 创建TMap类型属性控件 */
	UWidget* CreateMapPropertyWidget(FMapProperty* MapProperty, void* PropertyValue);

	/** 创建USTRUCT类型属性控件 */
	UWidget* CreateStructPropertyWidget(FStructProperty* StructProperty, void* PropertyValue);

	/** 创建属性名和值的行 */
	UWidget* CreatePropertyRow(const FString& PropertyName, UWidget* ValueWidget, bool bIsExpandable = false);

	/** 处理浮点值变化 */
	UFUNCTION()
	void HandleFloatValueChanged(float NewValue);

	/** 处理整型值变化 */
	UFUNCTION()
	void HandleIntValueChanged(int32 NewValue);

	/** 处理字符串值变化 */
	UFUNCTION()
	void HandleStringValueChanged(const FText& NewText, ETextCommit::Type CommitType);

	/** 处理布尔值变化 */
	UFUNCTION()
	void HandleBoolValueChanged(bool NewValue);

	/** 处理枚举值变化 */
	UFUNCTION()
	void HandleEnumValueChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

private:
	/** 当前编辑的对象 */
	UPROPERTY(Transient)
	UObject* TargetObject;

	/** 存储控件与属性名称的映射关系 */
	TMap<UWidget*, FName> WidgetToPropertyMap;

	/** 模板类：文本标签 */
	UPROPERTY(Transient)
	TSubclassOf<UUserWidget> LabelTemplateClass;
	
	/** 模板类：float值的SpinBox */
	UPROPERTY(Transient)
	TSubclassOf<USpinBox> FloatSpinBoxTemplateClass;

	/** 模板类：int值的SpinBox */
	UPROPERTY(Transient)
	TSubclassOf<USpinBox> IntSpinBoxTemplateClass;

	/** 模板类：文本框 */
	UPROPERTY(Transient)
	TSubclassOf<UEditableTextBox> TextBoxTemplateClass;

	/** 模板类：复选框 */
	UPROPERTY(Transient)
	TSubclassOf<UCheckBox> CheckBoxTemplateClass;

	/** 模板类：下拉框（枚举用） */
	UPROPERTY(Transient)
	TSubclassOf<UComboBoxString> ComboBoxTemplateClass;

	/** 创建或使用模板创建控件 */
	template<class WidgetClass>
	WidgetClass* CreateWidgetFromTemplate(TSubclassOf<WidgetClass> TemplateClass);

	/** 获取属性显示名称 */
	FString GetPropertyDisplayName(FProperty* Property) const;

	/** 获取控件对应的属性名称 */
	FName GetPropertyNameFromWidget(UWidget* Widget) const;

	/** 设置控件的属性名称 */
	void SetWidgetPropertyName(UWidget* Widget, FName PropertyName);

	/** 根据属性更新对象值 */
	void UpdatePropertyValue(FName PropertyName, const void* NewValue, FProperty* Property = nullptr);

	/** 通知属性改变 */
	void NotifyPropertyChanged(FName PropertyName, FProperty* Property = nullptr);
}; 