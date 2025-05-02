// Fill out your copyright notice in the Description page of Project Settings.


#include "PythonBridge.h"

UPythonBridge* UPythonBridge::Get()
{
	TArray<UClass*> PythonBridgeClasses;
	GetDerivedClasses(UPythonBridge::StaticClass(), PythonBridgeClasses);
	int32 NumClasses = PythonBridgeClasses.Num();
	if (NumClasses > 0)
	{
		return Cast<UPythonBridge>(PythonBridgeClasses[NumClasses - 1]->GetDefaultObject());
	}
	return nullptr;
}
