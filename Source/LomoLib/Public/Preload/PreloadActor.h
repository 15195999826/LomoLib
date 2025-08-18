// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PreloadActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPreloadDataReady);

UCLASS()
class LOMOLIB_API APreloadActor : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	APreloadActor();

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnPreloadDataReady OnPreloadDataReady;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
