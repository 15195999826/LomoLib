// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Animation/ProgramAnimationTypes.h"
#include "Animation/ProgramAnimationDataAsset.h"
#include "ProgramAnimationSettings.generated.h"

/**
 * 程序动画设置
 * 管理动画资产和生成的反转动画数据
 */
UCLASS(config=Game, DefaultConfig)
class LOMOLIB_API UProgramAnimationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UProgramAnimationSettings();

	virtual FName GetCategoryName() const override;

	// 动画资产扫描根目录
	UPROPERTY(config, EditAnywhere, Category = "Generate|Config", meta=(DisplayName="动画资产扫描目录"))
	TArray<FString> ProgramAnimationAssetRootDir;

	// 需要创建反转的动画
	UPROPERTY(config, EditAnywhere, Category = "Generate|Config", meta=(DisplayName="需要创建反转的动画", GetOptions="LomoLib.LomoLibPropertyHelper.GetProgramAnimations"))
	TArray<FName> NeedReverseProgramAnimations;

	// 自动生成的动画资产映射 (不要手动编辑)
	UPROPERTY(config, EditAnywhere, Category = "Generate|DontEdit", meta=(DisplayName="动画资产映射 (自动生成)"))
	TMap<FName, TSoftObjectPtr<UProgramAnimationDataAsset>> ProgramAnimationAssets;

	// 自动生成的反转动画数据 (不要手动编辑)
	UPROPERTY(config, EditAnywhere, Category = "Generate|DontEdit", meta=(DisplayName="反转动画数据 (自动生成)"))
	TMap<FName, FProgramAnimationData> GeneratedReverseAnimations;

	/**
	 * 获取动画数据
	 * @param AnimationName 动画名称
	 * @return 动画数据，如果找不到返回nullptr
	 */
	static const FProgramAnimationData& GetAnimationData(const FName& AnimationName);
};