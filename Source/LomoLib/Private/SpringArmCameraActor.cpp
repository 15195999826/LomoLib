// Fill out your copyright notice in the Description page of Project Settings.



#include "SpringArmCameraActor.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "GameFramework/SpringArmComponent.h"


ASpringArmCameraActor::ASpringArmCameraActor(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	auto LocalCameraComponent = GetCameraComponent();
	LocalCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	LocalCameraComponent->bUsePawnControlRotation = false;

	CursorDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("CursorDecal"));
	CursorDecal->SetupAttachment(RootComponent);

	PrimaryActorTick.bCanEverTick = true;
}

void ASpringArmCameraActor::BindInput(APlayerController* InPlayerController)
{
	DefaultRotation = CameraBoom->GetComponentRotation();
	DefaultZoom = CameraBoom->TargetArmLength;
	CurrentZoom = DefaultZoom;
#if !WITH_EDITOR
	FTimerHandle OnWindowResizedTimerHandle;
	// 延迟0.1秒， 等待Viewport初始化完成
	GetWorld()->GetTimerManager().SetTimer(OnWindowResizedTimerHandle, [this]()
	{
		OnWindowResized();
	}, 0.1f, false);
#else
	OnWindowResized();
#endif
	// 监听WindowResized事件
	FCoreDelegates::OnSafeFrameChangedEvent.AddUObject(this, &ASpringArmCameraActor::OnWindowResized);

	// 如果是战斗关卡的玩家控制器
	if (InPlayerController && InPlayerController->IsLocalPlayerController())
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(InPlayerController->GetLocalPlayer());
		if (Subsystem)
		{
			Subsystem->AddMappingContext(BattleInputMappingContext, 0);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[ASpringArmCameraActor::SetupPlayerInputComponent] Subsystem is null"));
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InPlayerController->InputComponent))
		{
			EnhancedInputComponent->BindAction(RotateCameraAxisAction, ETriggerEvent::Triggered, this,
											   &ASpringArmCameraActor::OnRotateCamera);
			EnhancedInputComponent->BindAction(ResetCameraAction, ETriggerEvent::Started, this,
											   &ASpringArmCameraActor::OnResetCamera);
			EnhancedInputComponent->BindAction(ZoomCameraAction, ETriggerEvent::Triggered, this, &ASpringArmCameraActor::OnZoomCamera);
			EnhancedInputComponent->BindAction(MoveCameraAxisAction, ETriggerEvent::Triggered, this, &ASpringArmCameraActor::OnMoveCamera);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[ASpringArmCameraActor::SetupPlayerInputComponent] EnhancedInputComponent is null"));
		}
	}
}

void ASpringArmCameraActor::WatchPosition(const FVector& InPosition)
{
	OnResetCamera();
	DesiredLocation = InPosition;
	SetActorLocation(InPosition);
}

void ASpringArmCameraActor::BeginPlay()
{
	Super::BeginPlay();
}

void ASpringArmCameraActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	SetActorLocation(FMath::Lerp(CameraBoom->GetComponentLocation(), DesiredLocation, DeltaSeconds * TraceSpeed));
	// Lerp the camera to the desired position
	ApplyEdgeScrolling();
	
	if (FMath::IsNearlyEqual(CurrentZoom, DesiredZoom))
	{
		return;
	}
	
	CurrentZoom = FMath::Lerp(CurrentZoom, DesiredZoom, DeltaSeconds * ZoomSpeed);
	CameraBoom->TargetArmLength = CurrentZoom;
}

void ASpringArmCameraActor::OnMoveCamera(const FInputActionValue& InputActionValue)
{
	auto InputVector = InputActionValue.Get<FVector2D>();
	// Get the current yaw rotation of the camera boom
	FRotator CameraRotation = CameraBoom->GetComponentRotation();
	FRotator YawRotation(0, CameraRotation.Yaw, 0);

	// Calculate forward and right vectors
	FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// Calculate the desired movement direction
	FVector DesiredMovementDirection = (ForwardVector * InputVector.Y + RightVector * InputVector.X).GetSafeNormal();

	// Update the desired location based on the movement direction
	DesiredLocation += DesiredMovementDirection * MoveSpeed;
}

void ASpringArmCameraActor::OnZoomCamera(const FInputActionValue& Value)
{
	DesiredZoom = FMath::Clamp(
		CurrentZoom + Value.GetMagnitude() * -ZoomSpeed,
		MinimumZoom,
		MaximumZoom
	);
}

void ASpringArmCameraActor::OnRotateCamera(const FInputActionValue& Value)
{
	FRotator NewRotation = CameraBoom->GetComponentRotation();
	NewRotation.Yaw += Value.GetMagnitude() * RotateSpeed;
	CameraBoom->SetWorldRotation(NewRotation);
}

void ASpringArmCameraActor::OnResetCamera()
{
	CameraBoom->SetWorldRotation(DefaultRotation);
	// Reset the camera zoom
	DesiredZoom = DefaultZoom;
}

void ASpringArmCameraActor::ApplyEdgeScrolling()
{
	if (!EnableEdgeScrolling)
	{
		return;
	}
	
	float DirectionX = 0;
	float DirectionY = 0;
	
	FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());
	MousePosition = MousePosition * UWidgetLayoutLibrary::GetViewportScale(GetWorld());
	if (this->IsValidMousePosition(MousePosition, Top))
	{
		DirectionX = 1;
	}
	else if (IsValidMousePosition(MousePosition, Bottom))
	{
		DirectionX = -1;
	}
	
	if (IsValidMousePosition(FVector2D(MousePosition.Y, MousePosition.X), Right))
	{
		DirectionY = 1;
	}
	else if (IsValidMousePosition(FVector2D(MousePosition.Y, MousePosition.X), Left))
	{
		DirectionY = -1;
	}

	DesiredLocation = CameraBoom->GetComponentLocation() +  FVector(  DirectionX * MoveSpeed, DirectionY * MoveSpeed, 0.0f);
}

bool ASpringArmCameraActor::IsValidMousePosition(FVector2D Positions, const FVector4& Rules)
{
	const bool InputY = Positions.Y >= Rules.X && Positions.Y <= Rules.Y;
	const bool InputX = Positions.X >= Rules.Z && Positions.X <= Rules.W;
	return InputX && InputY;
}

void ASpringArmCameraActor::OnWindowResized()
{
	const FVector2D Result = UWidgetLayoutLibrary::GetViewportSize(GetWorld());

	Top = FVector4(0, 0, 0, 0);
	Bottom = FVector4(0, 0, 0, 0);
	Left = FVector4(0, 0, 0, 0);
	Right = FVector4(0, 0, 0, 0);

	Top.Y = (MovementZoneInPercent / 100.f) * Result.Y;
	Top.W = Result.X;

	Bottom.X = (1 - (MovementZoneInPercent / 100.f)) * Result.Y;
	Bottom.Y = Result.Y;
	Bottom.W = Result.X;

	Left.Y = (MovementZoneInPercent / 100.f) * Result.X;
	Left.W = Result.Y;

	Right.X = (1 - (MovementZoneInPercent / 100.f)) * Result.X;
	Right.Y = Result.X;
	Right.W = Result.Y;	
}

