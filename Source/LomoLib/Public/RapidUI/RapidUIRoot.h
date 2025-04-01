// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RapidUIRoot.generated.h"

class UOverlay;
class UAutoDWBattleWindow;
class UAutoDWMainWindow;

UENUM(BlueprintType)
enum class ERapidUIOverlayType : uint8
{
	BackEnd,
	Main,
	FrontEnd
};


/**
 * 
 */
UCLASS()
class LOMOLIB_API URapidUIRoot : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

protected:
	// Overlay, 自下而上， BackEnd , Main, FrontEnd
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UOverlay> BackEndOverlay;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UOverlay> MainOverlay;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UOverlay> FrontEndOverlay;

	// 将不显示的Widget放在这里
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UOverlay> HiddenOverlay;

	// Widget映射表，用于存储和查询所有已添加的Widget
	UPROPERTY()
	TMap<FName, TObjectPtr<UUserWidget>> WidgetMap;

public:
	UFUNCTION(BlueprintCallable, Category = "RapidUI")
	void PushUI(ERapidUIOverlayType OverlayType,
	            UUserWidget* Widget,
	            FName InWidgetName,
	            TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = EHorizontalAlignment::HAlign_Fill,
	            TEnumAsByte<EVerticalAlignment> VerticalAlignment = EVerticalAlignment::VAlign_Fill);

	// 通过名称查找Widget
	UFUNCTION(BlueprintCallable, Category = "RapidUI")
	UUserWidget* FindWidgetByName(const FName& InWidgetName);

	// 通过类型查找第一个匹配的Widget
	UFUNCTION(BlueprintCallable, Category = "RapidUI")
	UUserWidget* FindWidgetByClass(TSubclassOf<UUserWidget> WidgetClass) const;

	// 通过类型获取所有匹配的Widget
	UFUNCTION(BlueprintCallable, Category = "RapidUI")
	TArray<UUserWidget*> FindAllWidgetsByClass(TSubclassOf<UUserWidget> WidgetClass) const;

	// 移除指定名称的Widget
	UFUNCTION(BlueprintCallable, Category = "RapidUI")
	bool RemoveWidgetByName(const FName& WidgetName);

	// 注册已经存在的Widget到映射表中(用于手动管理)
	UFUNCTION(BlueprintCallable, Category = "RapidUI")
	void RegisterExistingWidget(UUserWidget* Widget, const FName& WidgetName);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowTips(const FText& InText, float Duration = 0.5f);
};
