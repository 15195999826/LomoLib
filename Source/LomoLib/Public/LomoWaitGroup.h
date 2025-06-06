#pragma once

#include "CoreMinimal.h"
#include "Async/Future.h"


class UWaitGroupManager;

DEFINE_LOG_CATEGORY_STATIC(LogWaitGroup, Log, All);

class LOMOLIB_API FLomoWaitGroup
{
public:
	// 声明完成事件的委托类型
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnWaitGroupCompleted, int32);
	friend UWaitGroupManager;
	
	FLomoWaitGroup(int32 InID) : ID(InID),Counter(0) {}
	
	void SetName(const FName& InName)
	{
		DebugName = InName;
	}
	
	// 增加等待计数
	void Add(int32 Delta = 1)
	{
		Counter += Delta;
	}

	// 完成一个任务
	void Done(const FName& DoneTaskDebugName = NAME_None)
	{
		UE_LOG(LogWaitGroup, Verbose, TEXT("[%s]Done ID: %d, DebugName:%s, Counter:%d"), 
		*DebugName.ToString(), ID, *DoneTaskDebugName.ToString(), Counter);
		if (--Counter == 0)
		{
			if (Promise.IsValid())
			{
				Promise->SetValue();
			}
			OnCompleted.Broadcast(ID);
		}
	}

	// 等待所有任务完成
	template <typename FunctorType>
	void Next(FunctorType&& InFunction)
	{
		if (Counter <= 0)
		{
			if (!bIsCancelled)
			{
				Forward<FunctorType>(InFunction)();
			}
			return;
		}

		Promise = MakeShared<TPromise<void>>();
		PendingFunction = Forward<FunctorType>(InFunction);
		Promise->GetFuture().Next([this](int32)
		{
			if (!bIsCancelled && PendingFunction)
			{
				PendingFunction();
			}
		});
	}

	const FName& GetDebugName() const
	{
		return DebugName;
	}

	
private:
	int32 ID;
	// 仅用于管理，不对外暴露
	FOnWaitGroupCompleted OnCompleted;
	FName DebugName;
	int32 Counter;
	TSharedPtr<TPromise<void>> Promise;

	TFunction<void()> PendingFunction;
	bool bIsCancelled = false;
};
