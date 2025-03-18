// Fill out your copyright notice in the Description page of Project Settings.


#include "RapidUI/Modal/RapidModalWidget.h"

void URapidModalWidget::PerformClose_Implementation(ERapidModalCloseResult Result, const FString& StrPayload)
{
	OnClose.Broadcast(this, Result, StrPayload);
}
