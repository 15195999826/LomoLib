// Fill out your copyright notice in the Description page of Project Settings.


#include "RapidUI/Modal/RapidAsyncModalTask.h"

#include "Kismet/GameplayStatics.h"

URapidAsyncModalTask* URapidAsyncModalTask::AsyncShowModalFromClass(const UObject* WorldContextObject,
                                                                  TSubclassOf<URapidModalWidget> Class, FRapidModalConfig InConfig)
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	URapidModalWidget* Widget = CreateWidget<URapidModalWidget>(
							  GameInstance, Class);
	Widget->Setup(InConfig);
	// Call default function
	return AsyncShowModalFromRef(WorldContextObject, Widget);
}

URapidAsyncModalTask* URapidAsyncModalTask::AsyncShowModalFromRef(const UObject* WorldContextObject,
	URapidModalWidget* ModalBaseWidget)
{
	URapidAsyncModalTask* BlueprintNode = NewObject<URapidAsyncModalTask>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->ModalRef = ModalBaseWidget;
	// Add to viewport
	ModalBaseWidget->AddToViewport();
	// Return
	return BlueprintNode;
}

void URapidAsyncModalTask::NativeOnClose(URapidModalWidget* UserWidget, ERapidModalCloseResult Result, const FString& StrPayload)
{
	OnClose.Broadcast(UserWidget, Result, StrPayload);
	SetReadyToDestroy();
}

void URapidAsyncModalTask::Activate()
{
	Super::Activate();

	// Bind
	ModalRef->OnClose.AddDynamic(this, &URapidAsyncModalTask::NativeOnClose);

	// Call on show event for custom stuff
	OnShow.Broadcast(ModalRef, ERapidModalCloseResult::Unknown, TEXT(""));
}

UWorld* URapidAsyncModalTask::GetWorld() const
{
	return WorldContextObject->GetWorld();
}
