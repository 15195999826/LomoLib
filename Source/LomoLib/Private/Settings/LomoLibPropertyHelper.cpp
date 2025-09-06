// Fill out your copyright notice in the Description page of Project Settings.

#include "Settings/LomoLibPropertyHelper.h"
#include "Settings/ProgramAnimationSettings.h"

TArray<FName> ULomoLibPropertyHelper::GetProgramAnimations()
{
	TArray<FName> AnimationNames;
	
	const auto* Settings = GetDefault<UProgramAnimationSettings>();
	if (Settings)
	{
		// 从配置的动画资产中获取所有动画名称
		for (const auto& AnimationPair : Settings->ProgramAnimationAssets)
		{
			AnimationNames.Add(AnimationPair.Key);
		}
	}
	
	// 按字母顺序排序
	AnimationNames.Sort([](const FName& A, const FName& B) {
		return A.ToString() < B.ToString();
	});
	
	return AnimationNames;
}