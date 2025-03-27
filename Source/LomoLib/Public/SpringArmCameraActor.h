// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Camera/CameraActor.h"
#include "SpringArmCameraActor.generated.h"

class UInputAction;
class UInputMappingContext;
/**
 * 
 */
UCLASS()
class LOMOLIB_API ASpringArmCameraActor : public ACameraActor
{
	GENERATED_BODY()

public:
	ASpringArmCameraActor(const FObjectInitializer& ObjectInitializer);
	
protected:
	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Camera)
	TObjectPtr<class USpringArmComponent> CameraBoom;

public:
	void BindInput(APlayerController* InPlayerController);

	UFUNCTION(BlueprintImplementableEvent)
	void OnHitGround(const FVector_NetQuantize& Location);
	UFUNCTION(BlueprintImplementableEvent)
	void OnHitActor(AActor* Actor);

	void WatchPosition(const FVector& InPosition);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	// Input Start	
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Cursor)
	TObjectPtr<UDecalComponent> CursorDecal;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TheOne Enhanced Camera | Inputs")
	TObjectPtr<UInputMappingContext> BattleInputMappingContext;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TheOne Enhanced Camera | Inputs")
	TObjectPtr<UInputAction> RotateCameraAxisAction;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TheOne Enhanced Camera | Inputs")
	TObjectPtr<UInputAction> ResetCameraAction;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TheOne Enhanced Camera | Inputs")
	TObjectPtr<UInputAction> MoveCameraAxisAction;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TheOne Enhanced Camera | Inputs")
	TObjectPtr<UInputAction> ZoomCameraAction;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TheOne Enhanced Camera | Inputs")
	TObjectPtr<UInputAction> CtrlPressingAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TheOne Enhanced Camera | Movement Settings")
	float DesiredZoom = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TheOne Enhanced Camera | Movement Settings")
	float ZoomSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TheOne Enhanced Camera | Movement Settings")
	float MinimumZoom = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TheOne Enhanced Camera | Movement Settings")
	float MaximumZoom = 1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TheOne Enhanced Camera | Movement Settings")
	float MoveSpeed = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TheOne Enhanced Camera | Movement Settings")
	float TraceSpeed = 1.50f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TheOne Enhanced Camera | Movement Settings")
	float RotateSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TheOne Enhanced Camera | Movement Settings")
	FVector DesiredLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TheOne Enhanced Camera | Movement Settings")
	bool EnableEdgeScrolling = true;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "TheOne Enhanced Camera | Movement Settings",
		meta=(EditCondition="EnableEdgeScrolling")
		)
	float MovementZoneInPercent = 5;

private:
	float CurrentZoom;
	FVector4 Top = FVector4();
	FVector4 Bottom = FVector4();
	FVector4 Left = FVector4();
	FVector4 Right = FVector4();

	bool bInputSetup = false;
	FRotator DefaultRotation;
	float DefaultZoom;
protected:
	UFUNCTION(BlueprintCallable)
	void OnResetCamera();
private:
	void OnMoveCamera(const FInputActionValue& InputActionValue);
	void OnZoomCamera(const FInputActionValue& Value);
	void OnRotateCamera(const FInputActionValue& Value);
	
	void ApplyEdgeScrolling();
	bool IsValidMousePosition(FVector2D Positions, const FVector4& Rules);
	void OnWindowResized();
};
