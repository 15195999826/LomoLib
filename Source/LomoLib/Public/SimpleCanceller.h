// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ICancellable.h"
#include "SimpleCanceller.generated.h"

/**
 * Simple implementation of ICancellable for testing purposes
 * Provides basic cancel/reset functionality that can be used in blueprints
 */
UCLASS(BlueprintType, Blueprintable)
class LOMOLIB_API USimpleCanceller : public UObject, public ICancellable
{
	GENERATED_BODY()

public:
	USimpleCanceller();

	// ICancellable interface implementation
	virtual bool IsCancelled() const override;

	/**
	 * Cancel the operation
	 * Sets the cancelled state to true
	 */
	UFUNCTION(BlueprintCallable, Category = "LomoLib|Test")
	void Cancel();

	/**
	 * Reset the cancelled state
	 * Sets the cancelled state to false
	 */
	UFUNCTION(BlueprintCallable, Category = "LomoLib|Test")
	void Reset();

	/**
	 * Get the current cancelled state
	 * @return true if cancelled, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "LomoLib|Test")
	bool GetIsCancelled() const;

private:
	/** Whether this canceller is in cancelled state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
	bool bIsCancelled;
};