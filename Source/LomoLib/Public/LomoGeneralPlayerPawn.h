// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LomoGeneralPlayerPawn.generated.h"

#define DEFINE_PLAYER_ATTRIBUTE(PropertyType, PropertyName) \
protected: \
PropertyType PropertyName; \
public: \
UPROPERTY(BlueprintAssignable) \
FProjectJ##PropertyName##ChangeSignature On##PropertyName##Change; \
FORCEINLINE PropertyType Get##PropertyName() const { return PropertyName; } \
FORCEINLINE void Set##PropertyName(PropertyType NewVal) \
{ \
if (PropertyName == NewVal) return; \
PropertyType OldVal = PropertyName; \
PropertyName = NewVal; \
On##PropertyName##Change.Broadcast(OldVal, NewVal); \
}

UCLASS()
class LOMOLIB_API ALomoGeneralPlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ALomoGeneralPlayerPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneComponent> SceneRoot;
	
	// Define player attributes Template
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FProjectJTestAttrChangeSignature, int32, OldVal, int32, NewVal);
	DEFINE_PLAYER_ATTRIBUTE(int32, TestAttr)
};
