// Fill out your copyright notice in the Description page of Project Settings.


#include "LomoPlayerInput.h"

#include "LomoLib.h"

void ULomoPlayerInput::EvaluateInputDelegates(const TArray<UInputComponent*>& InputComponentStack,
	const float DeltaTime, const bool bGamePaused, const TArray<TPair<FKey, FKeyState*>>& KeysWithEvents)
{
	if (DebugKeyConsume)
	{
		PreviousConsumedStates.Empty();
		// 记录调用前的消费状态
		for (const TPair<FKey, FKeyState*>& KeyWithEvent : KeysWithEvents)
		{
			PreviousConsumedStates.Add(KeyWithEvent.Key, KeyWithEvent.Value->bConsumed);
		}
	}
	
	// 调用父类方法
	Super::EvaluateInputDelegates(InputComponentStack, DeltaTime, bGamePaused, KeysWithEvents);

	if (DebugKeyConsume)
	{
		// 检查哪些键在调用后状态发生了变化
		for (const TPair<FKey, FKeyState*>& KeyWithEvent : KeysWithEvents)
		{
			const FKey& Key = KeyWithEvent.Key;
			FKeyState* KeyState = KeyWithEvent.Value;
        
			// 获取之前的状态
			bool bWasConsumed = PreviousConsumedStates[Key];
        
			// 检查状态是否从未消费变成已消费
			if (!bWasConsumed && KeyState->bConsumed)
			{
				// 尝试确定哪个组件消费了此键
				UE_LOG(LogLomoLib, Log, TEXT("Key %s was consumed"), *Key.ToString());
            
				// 检查每个输入组件，看看是谁可能消费了按键
				for (int32 i = 0; i < InputComponentStack.Num(); ++i)
				{
					UInputComponent* IC = InputComponentStack[i];
					if (IC && IC->HasBindings())
					{
						// 检查键绑定
						for (const FInputKeyBinding& KeyBinding : IC->KeyBindings)
						{
							if (KeyBinding.Chord.Key == Key)
							{
								AActor* Owner = IC->GetOwner<AActor>();
								FString OwnerName = Owner ? Owner->GetName() : TEXT("Unknown");
								UE_LOG(LogLomoLib, Log, TEXT("  - Possible consumer: %s (InputComponent priority: %d)"), 
									*OwnerName, i);
							}
						}
					}
				}
			}
		}
	}
}
