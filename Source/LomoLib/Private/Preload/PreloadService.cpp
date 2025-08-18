// Fill out your copyright notice in the Description page of Project Settings.


#include "Preload/PreloadService.h"

#include "Preload/GeneralPreloaderActor.h"
#include "Preload/PreloadProvider.h"

UPreloadService* UPreloadService::Instance = nullptr;


UPreloadService* UPreloadService::Get()
{
	if (!Instance)
	{
		Instance = NewObject<UPreloadService>();
		Instance->AddToRoot(); // 防止被垃圾回收
	}
	
	return Instance;
	
}

void UPreloadService::RegisterProvider(TScriptInterface<IPreloadProvider> Provider)
{
	PreloadProvider = Provider;
}

void UPreloadService::RegisterLoader(TObjectPtr<AGeneralPreloaderActor> InLoader)
{
	Loader = InLoader;
}

void UPreloadService::UnregisterProvider()
{
	PreloadProvider = nullptr;
}

void UPreloadService::StartPreload()
{
	if (PreloadProvider && Loader)
	{
		Loader->StartPreload(PreloadProvider->GetPreloadData());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PreloadProvider or Loader is not set in UPreloadService::StartPreload()"));
	}
}
