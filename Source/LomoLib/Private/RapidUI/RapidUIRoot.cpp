// Fill out your copyright notice in the Description page of Project Settings.


#include "RapidUI/RapidUIRoot.h"

#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"

void URapidUIRoot::NativeConstruct()
{
	Super::NativeConstruct();
}

void URapidUIRoot::PushUI(ERapidUIOverlayType OverlayType, UUserWidget* Widget, FName InWidgetName,
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment, TEnumAsByte<EVerticalAlignment> VerticalAlignment)
{
	// 检查Widget是否有效
	if (!Widget)
	{
		return;
	}

	// 添加到对应Overlay
	switch (OverlayType)
	{
	case ERapidUIOverlayType::BackEnd:
		BackEndOverlay->AddChild(Widget);
		break;
	case ERapidUIOverlayType::Main:
		MainOverlay->AddChild(Widget);
		break;
	case ERapidUIOverlayType::FrontEnd:
		FrontEndOverlay->AddChild(Widget);
		break;
	}
	
	auto OverlaySlot = CastChecked<UOverlaySlot>(Widget->Slot);
	OverlaySlot->SetHorizontalAlignment(HorizontalAlignment);
	OverlaySlot->SetVerticalAlignment(VerticalAlignment);

	// 存储到映射表中，如果已存在则更新
	if (WidgetMap.Contains(InWidgetName))
	{
		UE_LOG(LogTemp, Warning, TEXT("[PushUI] Widget with name %s already exists. 替换了引用."), *InWidgetName.ToString());
	}
	
	WidgetMap.Add(InWidgetName, Widget);
}

UUserWidget* URapidUIRoot::FindWidgetByName(const FName& InWidgetName)
{
	if (WidgetMap.Contains(InWidgetName))
	{
		return WidgetMap[InWidgetName];
	}	
	return nullptr;
}

UUserWidget* URapidUIRoot::FindWidgetByClass(TSubclassOf<UUserWidget> WidgetClass) const
{
	// 遍历映射表，查找第一个匹配类型的Widget
	for (const TPair<FName, TObjectPtr<UUserWidget>>& Pair : WidgetMap)
	{
		if (Pair.Value && Pair.Value->IsA(WidgetClass))
		{
			return Pair.Value;
		}
	}
	return nullptr;
}

TArray<UUserWidget*> URapidUIRoot::FindAllWidgetsByClass(TSubclassOf<UUserWidget> WidgetClass) const
{
	TArray<UUserWidget*> Result;
	
	// 遍历映射表，查找所有匹配类型的Widget
	for (const TPair<FName, TObjectPtr<UUserWidget>>& Pair : WidgetMap)
	{
		if (Pair.Value && Pair.Value->IsA(WidgetClass))
		{
			Result.Add(Pair.Value);
		}
	}
	
	return Result;
}

bool URapidUIRoot::RemoveWidgetByName(const FName& WidgetName)
{
	UUserWidget* Widget = FindWidgetByName(WidgetName);
	if (Widget)
	{
		// 从UI中移除
		Widget->RemoveFromParent();
		
		// 从映射表中移除
		WidgetMap.Remove(WidgetName);
		return true;
	}
	return false;
}

void URapidUIRoot::RegisterExistingWidget(UUserWidget* Widget, const FName& WidgetName)
{
	if (Widget)
	{
		WidgetMap.Add(WidgetName, Widget);
	}
}