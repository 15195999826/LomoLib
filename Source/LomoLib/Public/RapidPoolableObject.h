// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RapidPoolableObject.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URapidPoolableObject : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LOMOLIB_API IRapidPoolableObject
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/** Called when an object is returned to the pool */
	virtual void OnReturnToPool() = 0;
    
	/** Called when an object is retrieved from the pool */
	virtual void OnGetFromPool() = 0;

    virtual void SetPoolID(const FString& InID) = 0;

    virtual const FString& GetPoolID() const = 0;
};

/**
 * Template class to manage object pools for different types
 * Must be used with UObject-derived classes that implement IPoolableObject
 * 关于模板函数：
 * 1. C++ 模板会为每个不同的 T 类型生成完全不同的类
 * 2. if constexpr 会在编译时决定哪部分代码被包含，哪部分被排除
 * 3. 编译器确实会为不同类型的 TRapidObjectPool<T> 生成不同的源代码
 * 未考虑线程安全， 如果需要线程安全，IdCounter和两个容器都需要加锁
 */
template<typename T>
class TRapidObjectPool
{
    static_assert(TIsDerivedFrom<T, UObject>::Value, "T must be a UObject-derived class");
    // 检查是否实现了IRapidPoolableObject接口
    static_assert(TIsDerivedFrom<T, IRapidPoolableObject>::Value, "T must implement IRapidPoolableObject interface");

public:
    TRapidObjectPool(UObject* InOwner, TSubclassOf<T> InObjectClass, const FString& InTypePrefix,
                     int32 InInitialPoolSize = 5, int32 InGrowNum = 3)
        : TypePrefix(InTypePrefix)
          , GrowNum(InGrowNum)
          , Owner(InOwner)
          , ObjectClass(InObjectClass)
    {
        GrowPool(InInitialPoolSize);
    }

    /** Get an object from the pool */
    T* GetObject()
    {
        if (AvailableObjects.Num() == 0)
        {
            GrowPool(GrowNum);
        }

        T* Object = AvailableObjects.Pop();

        // Call the interface method if it's implemented
        IRapidPoolableObject* Interface = Cast<IRapidPoolableObject>(Object);
        // Generate a unique ID for this object if needed
        FString ObjectID = FString::Printf(TEXT("%s_%d"), *TypePrefix, IdCounter++);
        Interface->SetPoolID(ObjectID);
        Interface->OnGetFromPool();

        // 如果是Actor类型，在编辑器模式下设置文件夹路径
        if constexpr (TIsDerivedFrom<T, AActor>::Value)
        {
#if WITH_EDITOR
            if (AActor* ActorObject = Cast<AActor>(Object))
            {
                ActorObject->SetFolderPath(FName(*FString::Printf(TEXT("%s_Using"), *TypePrefix)));
            }
#endif
        }

        // 将对象添加到使用中的Map，以ID为键
        UsingObjects.Add(ObjectID, Object);
        return Object;
    }

    /** Return an object to the pool */
    void RecycleObject(T* Object)
    {
        if (!Object)
            return;

        // 添加更多有效性检查
        if (!Object || Object->IsPendingKill())
        {
            return;
        }
    
        // 检查对象是否已经在可用列表中
        if (AvailableObjects.Contains(Object))
        {
            UE_LOG(LogTemp, Warning, TEXT("Attempting to recycle an object that's already in the pool!"));
            return;
        }
        
        // Call the interface method if it's implemented
        IRapidPoolableObject* Interface = Cast<IRapidPoolableObject>(Object);
        Interface->OnReturnToPool();

        // 如果是Actor类型，在编辑器模式下设置文件夹路径
        if constexpr (TIsDerivedFrom<T, AActor>::Value)
        {
#if WITH_EDITOR
            if (AActor* ActorObject = Cast<AActor>(Object))
            {
                ActorObject->SetFolderPath(FName(*FString::Printf(TEXT("%s_Pool"), *TypePrefix)));
            }
#endif
        }

        // 从Map中移除对象
        UsingObjects.Remove(Interface->GetPoolID());
        AvailableObjects.Add(Object);
    }

    /** Get all currently used objects */
    TArray<T*> GetAllUsingObjects() const
    {
        TArray<T*> Result;
        UsingObjects.GenerateValueArray(Result);
        return Result;
    }

    /** Find a using object by its pool ID */
    T* FindUsingObjectByID(const FString& ID) const
    {
        return UsingObjects.FindRef(ID);
    }

    /** Clear the pool */
    void ClearPool()
    {
        AvailableObjects.Empty();
        UsingObjects.Empty();
    }

private:
    /** Create more objects for the pool */
    void GrowPool(int32 Count)
    {
        if (!ObjectClass)
            return;

        UWorld* World = Owner->GetWorld();
        if (!World)
            return;

        for (int32 i = 0; i < Count; i++)
        {
            T* CreatedObject = nullptr;

            // 检查T是否为Actor类型
            if constexpr (TIsDerivedFrom<T, AActor>::Value)
            {
                static FVector HiddenLocation = FVector(0, 0, -10000); // Location to hide objects when not in use
                // Actor类型，使用SpawnActor
                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                SpawnParams.ObjectFlags |= RF_Transient;

                CreatedObject = World->SpawnActor<T>(ObjectClass, HiddenLocation, FRotator::ZeroRotator, SpawnParams);
                if (CreatedObject)
                {
                    CreatedObject->SetActorHiddenInGame(true);
                    
#if WITH_EDITOR
                    // 在编辑器模式下为新创建的Actor设置初始文件夹路径
                    CreatedObject->SetFolderPath(FName(*FString::Printf(TEXT("%s_Pool"), *TypePrefix)));
#endif
                }
            }
            else
            {
                // 非Actor类型，使用NewObject
                CreatedObject = NewObject<T>(Owner.Get(), ObjectClass, NAME_None, RF_Transient);
            }

            if (CreatedObject)
            {
                AvailableObjects.Add(CreatedObject);
            }
        }
    }

    int32 IdCounter{0};
    FString TypePrefix;

    int32 GrowNum;
    TWeakObjectPtr<UObject> Owner;
    TSubclassOf<T> ObjectClass;
    TArray<T*> AvailableObjects;
    TMap<FString, T*> UsingObjects;
};
