// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PreloadProvider.generated.h"

class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FPreloadData
{
	GENERATED_BODY()

	FPreloadData()
	{
	}

	UPROPERTY(BlueprintReadOnly)
	TArray<TSoftObjectPtr<UNiagaraSystem>> NiagaraSystems;

	UPROPERTY(BlueprintReadOnly)
	TArray<TSoftObjectPtr<UMaterial>> Materials;

	// StaticMesh
	UPROPERTY(BlueprintReadOnly)
	TArray<TSoftObjectPtr<UStaticMesh>> StaticMeshes;

	UPROPERTY()
	int32 PerFrameLoadCount = 1;

	UPROPERTY()
	bool bPrintLog = false;
};

// This class does not need to be modified.
UINTERFACE()
class UPreloadProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LOMOLIB_API IPreloadProvider
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual FPreloadData GetPreloadData() = 0;
};
