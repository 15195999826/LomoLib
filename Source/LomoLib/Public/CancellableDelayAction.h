// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "ICancellable.h"

/**
 * A cancellable delay action that extends the standard FDelayAction
 * Checks cancellation status on each update and can terminate early if cancelled
 */
class LOMOLIB_API FCancellableDelayAction : public FPendingLatentAction
{
public:
	/**
	 * Constructor
	 * @param Duration The duration to delay in seconds
	 * @param LatentInfo The latent action info containing callback information
	 * @param InCancellableContext The object that implements ICancellable interface for cancellation checks
	 */
	FCancellableDelayAction(float Duration, const FLatentActionInfo& LatentInfo, TScriptInterface<ICancellable> InCancellableContext);

	/**
	 * Called each frame to update the delay action
	 * Checks cancellation status first, then standard time countdown
	 * @param Response The latent response object to signal completion
	 */
	virtual void UpdateOperation(FLatentResponse& Response) override;

#if WITH_EDITOR
	/**
	 * Get description for debugging in editor
	 * @return Description string for debugging purposes
	 */
	virtual FString GetDescription() const override;
#endif

private:
	/** Time remaining in seconds */
	float TimeRemaining;

	/** The execution function to call when complete */
	FName ExecutionFunction;

	/** The output link to trigger when complete */
	int32 OutputLink;

	/** The callback target object */
	FWeakObjectPtr CallbackTarget;

	/** The cancellable context for checking cancellation status */
	TWeakInterfacePtr<ICancellable> CancellableContext;
};