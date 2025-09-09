// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleCanceller.h"

USimpleCanceller::USimpleCanceller()
	: bIsCancelled(false)
{
}

bool USimpleCanceller::IsCancelled() const
{
	return bIsCancelled;
}

void USimpleCanceller::Cancel()
{
	bIsCancelled = true;
}

void USimpleCanceller::Reset()
{
	bIsCancelled = false;
}

bool USimpleCanceller::GetIsCancelled() const
{
	return bIsCancelled;
}