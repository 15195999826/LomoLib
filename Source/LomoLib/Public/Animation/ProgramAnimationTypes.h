// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "ProgramAnimationTypes.generated.h"

UENUM(BlueprintType)
enum class EProgramAnimationCurveType : uint8
{
	Linear		UMETA(DisplayName = "Linear"),
	EaseIn		UMETA(DisplayName = "EaseIn"),
	EaseOut		UMETA(DisplayName = "EaseOut"),
	EaseInOut	UMETA(DisplayName = "EaseInOut"),
	Bezier		UMETA(DisplayName = "Bezier")
};

UENUM(BlueprintType)
enum class EProgramAnimationPlayMode : uint8
{
	Once		UMETA(DisplayName = "Once"),
	Loop		UMETA(DisplayName = "Loop"),
};

UENUM(BlueprintType)
enum class EProgramAnimationAssetCreateResult : uint8
{
	Success		UMETA(DisplayName = "创建成功"),
	Overwritten	UMETA(DisplayName = "覆盖成功"),
	Failed		UMETA(DisplayName = "创建失败")
};

/**
 *
 * ● 📝 Complement模式解释
 *
 *  我用一个具体例子来说明：
 *
 *  🎯 场景：手牌悬停动画
 *  - HoverUp动画：从位置0弹到位置100（进度0→1）
 *  - HoverDown动画：从位置100回到位置0（进度0→1，但实际效果是1→0）
 *
 *  ⚡ Complement的含义
 *
 *  假设HoverUp动画播放到30%时被中断：
 *  - 当前SharedProgress = 0.3
 *  - 手牌实际位置在30（0到100的30%处）
 *
 *  此时播放HoverDown，如果使用Complement模式：
 *  - 新动画从进度 1 - 0.3 = 0.7 开始播放
 *  - 这意味着HoverDown动画跳过前70%，直接从70%开始
 *  - 手牌从当前位置30开始下降到0
 *
 */
// UENUM(BlueprintType)
// enum class EProgramAnimationStartMode : uint8
// {
// 	FromStart	UMETA(DisplayName = "FromStart"),
// 	Complement	UMETA(DisplayName = "Complement")
// };

/**
 * 锚点时间段结构体
 * 定义在特定时间段内使用的锚点位置
 */
USTRUCT(BlueprintType)
struct LOMOLIB_API FProgramAnimationAnchor
{
	GENERATED_BODY()

	FProgramAnimationAnchor() {}

	FProgramAnimationAnchor(float InStartTime, float InEndTime, const FVector& InAnchorLocation, bool bInRelativeToSelf = false)
		: StartTime(InStartTime), EndTime(InEndTime), AnchorLocation(InAnchorLocation), bRelativeToSelf(bInRelativeToSelf) {}

	// 锚点生效开始时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor")
	float StartTime = 0.0f;

	// 锚点生效结束时间  
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor")
	float EndTime = 1.0f;

	// 锚点世界坐标位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor")
	FVector AnchorLocation = FVector::ZeroVector;

	// 是否相对于Actor位置：false=世界坐标，true=相对于Actor位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor", meta = (DisplayName = "相对于Actor位置"))
	bool bRelativeToSelf = false;
};

USTRUCT(BlueprintType)
struct LOMOLIB_API FProgramAnimationKeyFrame
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float Time = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FTransform Transform = FTransform::Identity;

	// Todo: 透明度
	
	FProgramAnimationKeyFrame()
	{
		Time = 0.0f;
		Transform = FTransform::Identity;
	}
};

USTRUCT(BlueprintType)
struct LOMOLIB_API FProgramAnimationEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float TriggerTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FString EventName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FString EventData;

	FProgramAnimationEvent()
	{
		TriggerTime = 0.0f;
	}
};

USTRUCT()
struct LOMOLIB_API FCurveValueLookup
{
	GENERATED_BODY()

	FCurveValueLookup(): MinTime(0), MaxTime(0), TimeStep(0)
	{
	}

	FCurveValueLookup(const FRichCurve& Curve, float InMinTime, float InMaxTime, float InTimeStep = 0.01f)
	{
		Initialize(Curve, InMinTime, InMaxTime, InTimeStep);
	}

private:
	TArray<float> ValueTable;
	float MinTime, MaxTime, TimeStep;

	void Initialize(const FRichCurve& Curve, float InMinTime, float InMaxTime, float InTimeStep = 0.01f)
	{
		MinTime = InMinTime;
		MaxTime = InMaxTime;
		TimeStep = InTimeStep;

		int32 NumSamples = FMath::CeilToInt((MaxTime - MinTime) / TimeStep) + 1;
		ValueTable.SetNum(NumSamples);

		for (int32 i = 0; i < NumSamples; i++)
		{
			float Time = MinTime + i * TimeStep;
			ValueTable[i] = Curve.Eval(Time);
		}
	}

public:
	float FindTimeForValue(float TargetValue, float Tolerance = 0.01f) const
	{
		// 找到第一个误差在容差范围内的值
		for (int32 i = 0; i < ValueTable.Num(); i++)
		{
			float Error = FMath::Abs(ValueTable[i] - TargetValue);
			if (Error <= Tolerance)
			{
				return MinTime + i * TimeStep;
			}
		}

		// 如果没找到容差范围内的，返回误差最小的
		int32 BestIndex = 0;
		float BestError = FMath::Abs(ValueTable[0] - TargetValue);

		for (int32 i = 1; i < ValueTable.Num(); i++)
		{
			float Error = FMath::Abs(ValueTable[i] - TargetValue);
			if (Error < BestError)
			{
				BestError = Error;
				BestIndex = i;
			}
		}

		return MinTime + BestIndex * TimeStep;
	}
};
  
USTRUCT(BlueprintType)
struct FProgramAnimationData
{
	GENERATED_BODY()

	FProgramAnimationData()
	{
		
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FName AnimationName = NAME_None;

	// 移动曲线表示从当前位置移动到目标位置的百分比
	// X轴总是0~1, 然后映射为百分比
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=ProgramMove, meta=(DisplayName="程序移动曲线", EditCondition= "!bUseAnchorMode", EditConditionHides))
	FRuntimeFloatCurve ProgramMoveCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=ProgramMove, meta=(DisplayName="程序移动开始时间", EditCondition= "!bUseAnchorMode", EditConditionHides))
	float ProgramMoveStartTime = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=ProgramMove, meta=(DisplayName="程序移动时长", EditCondition= "!bUseAnchorMode", EditConditionHides))
	float ProgramMoveDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	float Duration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	EProgramAnimationCurveType CurveType = EProgramAnimationCurveType::Linear;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TArray<FProgramAnimationKeyFrame> KeyFrames;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	EProgramAnimationPlayMode PlayMode = EProgramAnimationPlayMode::Once;

	// Todo: 始终相对初始Transform变换， false情况暂未测试, 意义上它是绝对数值， 比如Scale， 目前看起来不需要用到, 禁止编辑
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bRelativeToStartTransform = true;

	// 锚点模式相关配置
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anchor", meta = (DisplayName = "启用锚点模式"))
	bool bUseAnchorMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anchor", meta = (DisplayName = "锚点时间段", EditCondition= "bUseAnchorMode", EditConditionHides))
	TArray<FProgramAnimationAnchor> AnchorTimeSegments;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anchor", meta = (DisplayName = "默认锚点位置", EditCondition= "bUseAnchorMode", EditConditionHides))
	FVector DefaultAnchorLocation = FVector::ZeroVector;
	
	// 运行时的一些参数， 不可配置
	UPROPERTY(Transient)
	bool bIsValid = false;

	// 静态工具函数：从正向动画生成反向动画
	static FProgramAnimationData GenerateReverseAnimation(const FProgramAnimationData& ForwardData)
	{
		FProgramAnimationData ReverseData = ForwardData;

		// 命名为 {ForwardData.AnimationName}_Reverse
		ReverseData.AnimationName = FName(*FString::Printf(TEXT("%s_Reverse"), *ForwardData.AnimationName.ToString()));
		
		// 计算反转的程序移动开始时间
		// 反转后：新的开始时间 = 总时长 - 原始(开始时间 + 移动时长)
		float OriginalMoveEndTime = ForwardData.ProgramMoveStartTime + ForwardData.ProgramMoveDuration;
		ReverseData.ProgramMoveStartTime = ForwardData.Duration - OriginalMoveEndTime;
		
		// 修正浮点数精度问题：确保 ProgramMoveStartTime + ProgramMoveDuration <= Duration
		float NewMoveEndTime = ReverseData.ProgramMoveStartTime + ReverseData.ProgramMoveDuration;
		if (NewMoveEndTime > ForwardData.Duration)
		{
			// 如果超出，调整 ProgramMoveStartTime
			ReverseData.ProgramMoveStartTime = FMath::Max(0.0f, ForwardData.Duration - ReverseData.ProgramMoveDuration);
		}
		
		// 反转所有KeyFrames的时间
		for (auto& KeyFrame : ReverseData.KeyFrames)
		{
			KeyFrame.Time = ForwardData.Duration - KeyFrame.Time;
		}
		
		// 反转KeyFrames数组顺序
		Algo::Reverse(ReverseData.KeyFrames);
		
		// 转换曲线类型：EaseIn <-> EaseOut
		switch (ForwardData.CurveType)
		{
			case EProgramAnimationCurveType::EaseIn:
				ReverseData.CurveType = EProgramAnimationCurveType::EaseOut;
			break;
			case EProgramAnimationCurveType::EaseOut:
				ReverseData.CurveType = EProgramAnimationCurveType::EaseIn;
			break;
			default:
				// Linear, EaseInOut, Bezier保持不变
					break;
		}
		
		// 移动曲线也需要反转（如果有的话）
		if (ForwardData.ProgramMoveCurve.GetRichCurveConst()->Keys.Num() > 0)
		{
			// 创建反转的移动曲线
			FRuntimeFloatCurve ReverseCurve;
			const FRichCurve* OriginalCurve = ForwardData.ProgramMoveCurve.GetRichCurveConst();
			
			for (auto It = OriginalCurve->GetKeyIterator(); It; ++It)
			{
				const FRichCurveKey& Key = *It;
				// 时间反转，值也反转 (0变1, 1变0)
				float NewTime = 1.0f - Key.Time;
				float NewValue = 1.0f - Key.Value;
				ReverseCurve.GetRichCurve()->AddKey(NewTime, NewValue);
			}
			
			ReverseData.ProgramMoveCurve = ReverseCurve;
		}

		// Todo: 锚点模式暂不考虑支持反转
		// 处理锚点相关数据
		// ReverseData.bUseAnchorMode = ForwardData.bUseAnchorMode;
		// ReverseData.DefaultAnchorLocation = ForwardData.DefaultAnchorLocation;
		//
		// // 反转锚点时间段的时间
		// ReverseData.AnchorTimeSegments.SetNum(ForwardData.AnchorTimeSegments.Num());
		// for (int32 i = 0; i < ForwardData.AnchorTimeSegments.Num(); i++)
		// {
		// 	const FProgramAnimationAnchor& OriginalAnchor = ForwardData.AnchorTimeSegments[i];
		// 	FProgramAnimationAnchor& ReversedAnchor = ReverseData.AnchorTimeSegments[i];
		// 	
		// 	// 反转时间段
		// 	ReversedAnchor.StartTime = ForwardData.Duration - OriginalAnchor.EndTime;
		// 	ReversedAnchor.EndTime = ForwardData.Duration - OriginalAnchor.StartTime;
		// 	ReversedAnchor.AnchorLocation = OriginalAnchor.AnchorLocation; // 锚点位置保持不变
		// }
		//
		// // 反转锚点时间段数组顺序
		// Algo::Reverse(ReverseData.AnchorTimeSegments);
		
		return ReverseData;
	}
};