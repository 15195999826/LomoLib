// Fill out your copyright notice in the Description page of Project Settings.


#include "LomoGeneralPlayerPawn.h"

// Sets default values
ALomoGeneralPlayerPawn::ALomoGeneralPlayerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

// Called when the game starts or when spawned
void ALomoGeneralPlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALomoGeneralPlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

