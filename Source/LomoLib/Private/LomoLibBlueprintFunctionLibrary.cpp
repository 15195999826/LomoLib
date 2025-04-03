// Fill out your copyright notice in the Description page of Project Settings.


#include "LomoLibBlueprintFunctionLibrary.h"

#include "Framework/Application/NavigationConfig.h"

void ULomoLibBlueprintFunctionLibrary::EnableUINav()
{
	FNavigationConfig& NavConfig = *FSlateApplication::Get().GetNavigationConfig();
	NavConfig.bTabNavigation = true;
}

void ULomoLibBlueprintFunctionLibrary::DisableUINav()
{
	FNavigationConfig& NavConfig = *FSlateApplication::Get().GetNavigationConfig();
	NavConfig.bTabNavigation = false;
}
