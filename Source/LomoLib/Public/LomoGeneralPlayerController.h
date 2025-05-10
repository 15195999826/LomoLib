// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LomoGeneralPlayerController.generated.h"

class URegisterInputActionTask;
class ASpringArmCameraActor;

UENUM()
enum class ELomoMouseState : uint8
{
	Invalid = 0,
	Idle = 1,
	Press = 2,
	Pressing = 3,
	Release = 4
};

/**
 * 
 */
UCLASS()
class LOMOLIB_API ALomoGeneralPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALomoGeneralPlayerController();
	
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category=Debug)
	bool DebugKeyConsume = false;
	
	// Todo: 可以进行设置， 是否使用通用相机
	
	UPROPERTY()
	ELomoMouseState LeftMouseState = ELomoMouseState::Idle;

	UPROPERTY()
	ELomoMouseState RightMouseState = ELomoMouseState::Idle;

	// 用于重新计算命中地面的位置， 主要用于网格地面
	virtual void RemapHitLocation(FVector& HitLocation) {};

	virtual void CustomTick(float DeltaSeconds, bool OverWidget, bool IsHitGround, const FVector& HitGroundLocation, AActor* InHitActor, UPrimitiveComponent* InHitComponent) {};

private:
	void SampleMouseState(ELomoMouseState& MouseState, bool IsJustPressed, bool IsDown);

	// 可以在任何地方使用的玩家输入事件注册
protected:
	// Todo: 需要做到当前队列中存在某一个Consume消耗此输入时，其它Action不再触发， 用于实现游戏内不同情况下按ESC结果不同的情况
	TMap<int, int> InputID2IndexMap;
	TArray<URegisterInputActionTask*> InputTasks;

public:
	void RegisterInputAction(URegisterInputActionTask* InTask);
	void UnregisterInputAction(const int& InTaskID);

	ASpringArmCameraActor* GetSpringArmCamera() const
	{
		return SpringArmCamera.Get();
	}
	// 便捷使用通用相机
protected:
	bool bHasSpringCamera = false;
	TWeakObjectPtr<ASpringArmCameraActor> SpringArmCamera;
	// 自行在代码中调用, 场景中必须已经放置了相机才会生效
	UFUNCTION(BlueprintCallable)
	void UseSpringCamera(const FName& InCameraTag);
};
