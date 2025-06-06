// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RapidPoolableObject.h"
#include "GameFramework/Actor.h"
#include "GeneralPoolActor.generated.h"

UCLASS()
class LOMOLIB_API AGeneralPoolActor : public AActor, public IRapidPoolableObject
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGeneralPoolActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// ------- IRapidPoolableObject Interface Start -------
	virtual void OnReturnToPool() override;
	virtual void OnGetFromPool() override;

	virtual void SetPoolID(const FString& InID) override
	{
		PoolID = InID;
	}

	virtual const FString& GetPoolID() const override
	{
		return PoolID;
	}

private:
	UPROPERTY()
	FString PoolID;
	// ------- IRapidPoolableObject Interface End -------
};
