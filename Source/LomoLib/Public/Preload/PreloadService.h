// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PreloadService.generated.h"

class AGeneralPreloaderActor;
class IPreloadProvider;
/**
 * 
 */
UCLASS()
class LOMOLIB_API UPreloadService : public UObject
{
	GENERATED_BODY()

public:
	static UPreloadService* Get();

	void RegisterProvider(TScriptInterface<IPreloadProvider> Provider);
	
	void RegisterLoader(TObjectPtr<AGeneralPreloaderActor> InLoader);

	void UnregisterProvider();

	void StartPreload();
	
private:
	static UPreloadService* Instance;

	UPROPERTY()
	TScriptInterface<IPreloadProvider> PreloadProvider;

	UPROPERTY()
	TObjectPtr<AGeneralPreloaderActor> Loader;
};
