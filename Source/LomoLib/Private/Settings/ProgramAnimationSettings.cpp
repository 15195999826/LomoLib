// Fill out your copyright notice in the Description page of Project Settings.

#include "Settings/ProgramAnimationSettings.h"

UProgramAnimationSettings::UProgramAnimationSettings()
{
	// 设置默认扫描目录
	ProgramAnimationAssetRootDir.Add(TEXT("Data/Animations"));
}

FName UProgramAnimationSettings::GetCategoryName() const
{
	return FName("LomoLib");
}

const FProgramAnimationData* UProgramAnimationSettings::GetAnimationData(const FName& AnimationName) const
{
	// 1. 优先查找生成的反转动画
	if (const FProgramAnimationData* ReverseData = GeneratedReverseAnimations.Find(AnimationName))
	{
		return ReverseData;
	}

	// 2. 查找普通动画资产
	if (const TSoftObjectPtr<UProgramAnimationDataAsset>* AssetPtr = ProgramAnimationAssets.Find(AnimationName))
	{
		if (AssetPtr->IsValid())
		{
			UProgramAnimationDataAsset* DataAsset = AssetPtr->LoadSynchronous();
			if (DataAsset)
			{
				return &DataAsset->AnimationData;
			}
		}
	}

	// 3. 没有找到
	return nullptr;
}