// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/ProgramAnimationTypes.h"
#include "ProgramAniComponent.generated.h"

/**
 * 动画播放参数, 处理一些特殊初始化需求
 */
USTRUCT(BlueprintType)
struct LOMOLIB_API FProgramAnimationParams
{
	GENERATED_BODY()

	FProgramAnimationParams(){};

	FProgramAnimationParams(bool bInForceVisibleAtStart)
		: bForceVisibleAtStart(bInForceVisibleAtStart) {}

	static const FProgramAnimationParams Invalid;
	
	static const FProgramAnimationParams ForceShow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bForceVisibleAtStart = false;
};

/**
 * 通用程序动画组件
 * 设计解决方案：增加成员变量OwnerSceneComponent, 一个Actor上创建多个UProgramAniComponent, 每个组件绑定不同的SceneComponent
 * 注意事项: A卡牌正在播放动画, B卡牌先Attach到A， 再播放动画时, B使用KeepRelativeTransform可以解决B的一些动画问题 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LOMOLIB_API UProgramAniComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UProgramAniComponent();

	const FVector& GetCachedStartLocation() const { return CachedStartLocation; }

	const FVector& GetCachedEndLocation() const { return CachedEndLocation; }

	FSimpleMulticastDelegate OnAnimationEnd;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void BindToSceneComponent(USceneComponent* InComponent);
	
	// 播放命名动画（需要通过动画服务获取数据）
	UFUNCTION(BlueprintCallable, Category = "Program Animation")
	int32 PlayProgramAnimationByName(const FName& InAniName, 
					   const FVector& StartLocation, 
					   const FVector& EndLocation,
					   bool bRelativeToParent = false,
					   float StartProgress = 0.0f,
					   const FProgramAnimationParams& Params = FProgramAnimationParams());

	UFUNCTION(BlueprintCallable, Category = "Program Animation")
	int32 ReverseLastAnimation(bool bAssignNewEndLocation = false,
	                   const FVector& NewEndLocation = FVector::ZeroVector);
	
	/**
	 * 这个函数默认起点是当前位置, 终点是当前执行动画数据的StartLocation
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "Program Animation")
	int32 InterruptAndReverseCurrentAnimation(bool bCancelWaitGroup = false);

	/**
	 * 打断并回滚当前动画
	 * 在关键帧改变位置时， 慎用这个函数, EndLocation的位置需要好好计算
	 * @param InCurrentLocation StartLocation虽然提供了入参， 但一般使用当前位置计算
	 * @param EndLocation 回滚时的目标位置, 可以与之前动画的StartLocation不同
	 * @param bCancelWaitGroup
	 * @return 
	 */
	int32 InterruptAndReverseCurrentAnimation(const FVector& InCurrentLocation,
	                                          const FVector& EndLocation,
	                                          bool bCancelWaitGroup = false);

	/**
	 * 直接播放动画数据
	 * 注意区分bRelativeToParent==true时的StartLocation和EndLocation入参问题
	 * @param AnimationData 
	 * @param StartLocation 当bRelativeToParent为true时，输入相对坐标; 否则输入世界坐标
	 * @param EndLocation 当bRelativeToParent为true时，输入相对坐标; 否则输入世界坐标
	 * @param bRelativeToParent 表示动画是否总是相对于父节点位置, 为True时，表现上，它会始终跟随父Actor的位置和旋转变化; 否则完全不受父节点影响
	 * @param StartProgress 
	 * @param bAutoInitialCache
	 * @param Params 处理一些特殊初始化需求
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "Program Animation")
	int32 PlayProgramAnimation(const FProgramAnimationData& AnimationData, 
	                   const FVector& StartLocation, 
	                   const FVector& EndLocation,
	                   bool bRelativeToParent,
	                   float StartProgress = 0.0f,
	                   bool bAutoInitialCache = true,
	                   const FProgramAnimationParams& Params = FProgramAnimationParams());

	UFUNCTION(BlueprintCallable, Category = "Program Animation")
	void StopAnimation();

	UFUNCTION(BlueprintPure, Category = "Program Animation")
	bool IsPlayingAnimation() const { return bIsPlayingAnimation; }

	UFUNCTION(BlueprintPure, Category = "Program Animation")
	bool IsPlayingReverseAnimation() const;

	FName GetCurrentAnimationName() const
    {
        return bIsPlayingAnimation ? CurrentAnimationData.AnimationName : NAME_None;
    }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	USceneComponent* GetTargetTransformComponent() const;

private:
	UPROPERTY()
	TWeakObjectPtr<USceneComponent> OwnerSceneComponent;

	// 播放状态
	UPROPERTY()
	bool bIsPlayingAnimation = false;

	// 动画数据缓存
	UPROPERTY()
	FProgramAnimationData CurrentAnimationData;

	// Transform缓存
	FTransform CachedInitialTransform = FTransform::Identity;
	FVector CachedStartLocation;
	FVector CachedEndLocation;

	// 时间追踪
	float AnimationStartTime = 0.0f;
	FTimerHandle AnimationTimerHandle;
	int HoveringWaitGroupID;

	// 相对坐标模式
	bool bIsRelativeToParent = false;

	// 上一次播放的动画的数据缓存
	bool bLastRelativeToParent = false;
	FTransform CachedLastAnimationInitialTransform = FTransform::Identity;

	// 锚点模式相关
	bool bIsAnchorMode = false;
	
	// 缓存的锚点时间段（启动时计算好的实际世界坐标）
	TArray<FProgramAnimationAnchor> CachedAnchorSegments;

	// 内部更新函数
	void UpdateAnimation();

	// 工具函数
	float GetProgramMoveAlpha(float ElapsedTime) const;
	FTransform GetKeyFrameTransform(float ElapsedTime, const FProgramAnimationData& AnimationData) const;
	float ApplyCurveType(float Alpha, EProgramAnimationCurveType CurveType) const;
	
	// Smooth插值相关函数
	FVector VectorHermiteInterpolate(const FVector& P0, const FVector& P1, const FVector& T0, const FVector& T1, float Alpha) const;
	float CalculateKeyFrameTangent(const TArray<FProgramAnimationKeyFrame>& KeyFrames, int32 KeyIndex, bool bIsInTangent, int32 ComponentIndex) const;
	
	FTransform CalculateFinalTransform() const;
	FVector GetProgramMoveLocation(float ElapsedTime) const;

	/**
	 * 模拟计算理论起始位置
	 * @param AnimationData 动画数据
	 * @param DesiredEndLocation 期望的终点位置
	 * @param CurrentPosition 当前实际位置
	 * @param CurrentProgressInNewAnim 当前位置在新动画中对应的进度
	 * @return 计算出的理论起始位置
	 */
	FVector SimulateStartTransform(const FProgramAnimationData& AnimationData,
	                              const FVector& DesiredEndLocation,
	                              const FVector& CurrentPosition,
	                              float CurrentProgressInNewAnim) const;

	// 锚点模式相关函数
	/**
	 * 根据动画时间获取当前锚点位置
	 * @param ElapsedTime 动画已播放时间
	 * @return 当前时间点应使用的锚点位置
	 */
	FVector GetCurrentAnchorLocation(float ElapsedTime) const;

	/**
	 * 锚点模式的Transform计算
	 * @param ElapsedTime 动画已播放时间
	 * @return 锚点模式下的最终Transform
	 */
	FTransform CalculateAnchorModeTransform(float ElapsedTime) const;

	/**
	 * 缓存锚点位置（在动画开始时调用，基于初始Actor位置计算）
	 */
	void CacheAnchorLocations();
};