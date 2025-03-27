// Fill out your copyright notice in the Description page of Project Settings.


#include "RapidUI/RapidIntPayloadSwitchBtn.h"

void URapidIntPayloadSwitchBtn::NativeConstruct()
{
	Super::NativeConstruct();
	OnBtnClicked.AddDynamic(this, &URapidIntPayloadSwitchBtn::NativeOnBtnClickedHandler);
}

void URapidIntPayloadSwitchBtn::NativeSetActive(bool bInActive)
{
	bActive = bInActive;
	OnGroupButtonActive.Broadcast(this, IntPayload, bActive);	
}

void URapidIntPayloadSwitchBtn::NativeOnBtnClickedHandler(int32 OutIntPayload)
{
	if (!CanClickInvoke)
	{
		return;
	}
	
	bActive = !bActive;
	OnGroupButtonActive.Broadcast(this, OutIntPayload, bActive);
}
