// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RapidModalWidget.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "RapidAsyncModalTask.generated.h"

/**
 * 
 */
UCLASS()
class LOMOLIB_API URapidAsyncModalTask : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable)
	FRapidModalEventSignature OnShow;
	
	UPROPERTY(BlueprintAssignable)
	FRapidModalEventSignature OnClose;
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
	Category = "LCC|UI")
	static URapidAsyncModalTask* AsyncShowModalFromClass(const UObject* WorldContextObject, TSubclassOf<URapidModalWidget> Class, FRapidModalConfig InConfig);
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"),
		Category = "LCC|UI")
	static URapidAsyncModalTask* AsyncShowModalFromRef(const UObject* WorldContextObject, URapidModalWidget* ModalBaseWidget);

	UFUNCTION()
	void NativeOnClose(URapidModalWidget* UserWidget, ERapidModalCloseResult Result, const FString& StrPayload);
	
	// UBlueprintAsyncActionBase interface
	virtual void Activate() override;
	//~UBlueprintAsyncActionBase interface

	// Allows the Object to use BP_Functions
	virtual class UWorld* GetWorld() const override;
	
private:
	const UObject* WorldContextObject;
	
	UPROPERTY()
	URapidModalWidget* ModalRef = nullptr;
};
