// Fill out your copyright notice in the Description page of Project Settings.

#include "CancellableDelayAction.h"

FCancellableDelayAction::FCancellableDelayAction(float Duration, const FLatentActionInfo& LatentInfo, TScriptInterface<ICancellable> InCancellableContext)
	: TimeRemaining(Duration)
	, ExecutionFunction(LatentInfo.ExecutionFunction)
	, OutputLink(LatentInfo.Linkage)
	, CallbackTarget(LatentInfo.CallbackTarget)
	, CancellableContext(InCancellableContext.GetInterface())
{
}

void FCancellableDelayAction::UpdateOperation(FLatentResponse& Response)
{
	// First check if we should cancel the operation
	if (CancellableContext.IsValid() && CancellableContext->IsCancelled())
	{
		// True cancellation: terminate the action without triggering the output pin
		// This means the nodes after the delay will NOT execute
		Response.DoneIf(true);
		return;
	}

	// Standard FDelayAction logic - countdown time
	TimeRemaining -= Response.ElapsedTime();
	Response.FinishAndTriggerIf(TimeRemaining <= 0.0f, ExecutionFunction, OutputLink, CallbackTarget);
}

#if WITH_EDITOR
FString FCancellableDelayAction::GetDescription() const
{
	return FString::Printf(TEXT("Cancellable Delay %.2f seconds remaining"), TimeRemaining);
}
#endif