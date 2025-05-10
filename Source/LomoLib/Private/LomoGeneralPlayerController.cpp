// Fill out your copyright notice in the Description page of Project Settings.


#include "LomoGeneralPlayerController.h"

#include "EngineUtils.h"
#include "SpringArmCameraActor.h"
#include "Widgets/SWidget.h"
// 注意，这个Include不能删除
#include "LomoLib.h"
#include "LomoPlayerInput.h"
#include "RegisterInputActionTask.h"
#include "Widgets/SViewport.h"

ALomoGeneralPlayerController::ALomoGeneralPlayerController()
{
	OverridePlayerInputClass = ULomoPlayerInput::StaticClass();
}

void ALomoGeneralPlayerController::BeginPlay()
{
	Super::BeginPlay();
	// 设置PlayerInput的DebugKeyConsume
	auto LomoPlayerInput = Cast<ULomoPlayerInput>(PlayerInput);
	if (LomoPlayerInput)
	{
		LomoPlayerInput->DebugKeyConsume = DebugKeyConsume;
	}
}

void ALomoGeneralPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	TSharedPtr ViewPort = FSlateApplication::Get().GetGameViewport();

	bool OverWidget = false;
	if (ViewPort.IsValid())
	{
		FWidgetPath WidgetUnderMouse = FSlateApplication::Get().LocateWindowUnderMouse(FSlateApplication::Get().GetCursorPos() , FSlateApplication::Get().GetInteractiveTopLevelWindows(), true);
		
		OverWidget = !(WidgetUnderMouse.IsValid() &&  WidgetUnderMouse.GetLastWidget() == ViewPort.ToSharedRef());
		// UE_LOG(LogTheOne, Log, TEXT("OverWidget: %d, %s"), OverWidget, *WidgetUnderMouse.GetLastWidget()->GetWidgetClass().GetWidgetType().ToString());
	}
	SampleMouseState(LeftMouseState, WasInputKeyJustPressed(EKeys::LeftMouseButton), IsInputKeyDown(EKeys::LeftMouseButton));
	SampleMouseState(RightMouseState, WasInputKeyJustPressed(EKeys::RightMouseButton), IsInputKeyDown(EKeys::RightMouseButton));
	FHitResult HitResult;
	bool HitGround = false;
	AActor* CanHitActor = nullptr;
	UPrimitiveComponent* HitComponent = nullptr;
	FVector HitGroundLocation;
	GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
	if (HitResult.bBlockingHit)
	{
		auto HitActor = HitResult.GetActor();
		if (HitActor)
		{
			if (HitResult.GetActor()->ActorHasTag("Ground"))
			{
				if (bHasSpringCamera)
				{
					HitGroundLocation = HitResult.Location;
					RemapHitLocation(HitGroundLocation);
					SpringArmCamera->OnHitGround(HitGroundLocation);
				}
				
				HitGround = true;
			}
			else if (HitResult.GetActor()->ActorHasTag("CanHit"))
			{
				if (bHasSpringCamera)
				{
					SpringArmCamera->OnHitActor(HitActor);
				}
				CanHitActor = HitActor;
			}
			// UE_LOG(LogLomoLib, Log, TEXT("HitActor: %s"), *HitActor->GetName());
		}

		HitComponent = HitResult.GetComponent();
	}
	
	CustomTick(DeltaSeconds, OverWidget, HitGround, HitGroundLocation, CanHitActor, HitComponent);
}

void ALomoGeneralPlayerController::SampleMouseState(ELomoMouseState& MouseState, bool IsJustPressed, bool IsDown)
{
	switch (MouseState)
	{
		case ELomoMouseState::Idle:
			if (IsJustPressed)
			{
				// UE_LOG(LogLomoLib, Log, TEXT("MouseState: Press"));
				MouseState = ELomoMouseState::Press;
			}
		break;
		case ELomoMouseState::Press:
			if (IsDown)
			{
				// UE_LOG(LogLomoLib, Log, TEXT("MouseState: Pressing"));
				MouseState = ELomoMouseState::Pressing;
			}
			else
			{
				// UE_LOG(LogLomoLib, Log, TEXT("MouseState: Release"));
				MouseState = ELomoMouseState::Release;
			}
		break;
		case ELomoMouseState::Pressing:
			if (!IsDown)
			{
				// UE_LOG(LogLomoLib, Log, TEXT("MouseState: Release"));
				MouseState = ELomoMouseState::Release;
			}
		break;
		case ELomoMouseState::Release:
				// UE_LOG(LogLomoLib, Log, TEXT("MouseState: Idle"));
				MouseState = ELomoMouseState::Idle;
		break;
		default: ;
	}
}

void ALomoGeneralPlayerController::RegisterInputAction(URegisterInputActionTask* InTask)
{
	if (InputComponent == nullptr)
	{
		UE_LOG(LogLomoLib, Error, TEXT("InputComponent is nullptr"));
		return;
	}

	if (InputID2IndexMap.Contains(InTask->TaskID))
	{
		UE_LOG(LogLomoLib, Error, TEXT("InputActionConfig.StrID is already registered"));
		return;
	}

	
	FInputKeyBinding KB(InTask->Key, InTask->InputEvent);

	KB.KeyDelegate.GetDelegateForManualSet().BindLambda([InTask]()
	{
		InTask->Delegate.Execute();
	});

	InputComponent->KeyBindings.Emplace(MoveTemp(KB));
	InputID2IndexMap.Add(InTask->TaskID, InputComponent->KeyBindings.Num() - 1);
	InputTasks.Add(InTask);
}

void ALomoGeneralPlayerController::UnregisterInputAction(const int& InTaskID)
{
	if (InputID2IndexMap.Contains(InTaskID))
	{
		int Index = InputID2IndexMap[InTaskID];
		InputComponent->KeyBindings.RemoveAt(Index);
		InputID2IndexMap.Remove(InTaskID);
		// 倒叙删除
		for (int i = InputTasks.Num() - 1; i >= 0; i--)
		{
			if (InputTasks[i]->TaskID == InTaskID)
			{
				InputTasks.RemoveAt(i);
				break;
			}
		}
		// 更新IndexMap中的数据， 当删除了一个元素后， 后续的元素Index会向前移动
		for (auto& Pair : InputID2IndexMap)
		{
			if (Pair.Value > Index)
			{
				Pair.Value--;
			}
		}
	}
}


void ALomoGeneralPlayerController::UseSpringCamera(const FName& InCameraTag)
{
	for (TActorIterator<ASpringArmCameraActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		ASpringArmCameraActor* CameraActor = *ActorItr;
		if (CameraActor->Tags.Contains(InCameraTag))
		{
			SpringArmCamera = CameraActor;
			CameraActor->BindInput(this);
			// 设置当前激活的相机
			this->SetViewTarget(CameraActor);
			break;
		}
	}

	bHasSpringCamera = SpringArmCamera != nullptr;
}