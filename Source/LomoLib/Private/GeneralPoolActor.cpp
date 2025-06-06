// Fill out your copyright notice in the Description page of Project Settings.


#include "GeneralPoolActor.h"


// Sets default values
AGeneralPoolActor::AGeneralPoolActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AGeneralPoolActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGeneralPoolActor::OnReturnToPool()
{
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
}

void AGeneralPoolActor::OnGetFromPool()
{
	SetActorEnableCollision(true);
	SetActorHiddenInGame(false);
}

