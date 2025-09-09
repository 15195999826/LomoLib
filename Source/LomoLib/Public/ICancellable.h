// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ICancellable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UCancellable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for objects that can be cancelled
 * Used by cancellable delay and other async operations
 */
class LOMOLIB_API ICancellable
{
	GENERATED_BODY()

public:
	/**
	 * Check if the operation should be cancelled
	 * @return true if the operation should be cancelled, false otherwise
	 */
	virtual bool IsCancelled() const = 0;
};