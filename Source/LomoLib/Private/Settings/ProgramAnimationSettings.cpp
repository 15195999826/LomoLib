// Fill out your copyright notice in the Description page of Project Settings.

#include "Settings/ProgramAnimationSettings.h"

#include "LomoLib.h"

UProgramAnimationSettings::UProgramAnimationSettings()
{

}

FName UProgramAnimationSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}

const FProgramAnimationData& UProgramAnimationSettings::GetAnimationData(const FName& AnimationName)
{
	auto Settings = GetDefault<UProgramAnimationSettings>();
	// 1. 优先查找生成的反转动画
	if (const FProgramAnimationData* ReverseData = Settings->GeneratedReverseAnimations.Find(AnimationName))
	{
		return *ReverseData;
	}

	// 2. 查找普通动画资产
	if (const TSoftObjectPtr<UProgramAnimationDataAsset>* AssetPtr = Settings->ProgramAnimationAssets.Find(AnimationName))
	{
		if (AssetPtr->IsValid())
		{
			UProgramAnimationDataAsset* DataAsset = AssetPtr->LoadSynchronous();
			if (DataAsset)
			{
				return DataAsset->AnimationData;
			}
		}
	}

	// 3. 没有找到
	static FProgramAnimationData InvalidData;
	UE_LOG(LogLomoLib, Warning, TEXT("[UProgramAnimationSettings] Animation data not found: %s"), *AnimationName.ToString());
	return InvalidData;
}