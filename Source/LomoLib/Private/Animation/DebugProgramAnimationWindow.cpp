// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/DebugProgramAnimationWindow.h"

#include "Components/ComboBoxString.h"
#include "Settings/ProgramAnimationSettings.h"

void UDebugProgramAnimationWindow::InitWindow()
{
	auto GSettings = GetDefault<UProgramAnimationSettings>();

	AnimationSelector->ClearOptions();
	AnimationSelector->AddOption("None");
	
	for (const auto& Tuple : GSettings->ProgramAnimationAssets)
	{
		AnimationSelector->AddOption(Tuple.Key.ToString());
	}
	
	AnimationSelector->SetSelectedIndex(0);
}

const FProgramAnimationData& UDebugProgramAnimationWindow::GetAnimationData(const FString& InAniName, bool bReverse)
{
	FString DstAniName = InAniName;
	if (bReverse)
	{
		// 增加_Reverse后缀
		DstAniName.Append(TEXT("_Reverse"));
	}

	return UProgramAnimationSettings::GetAnimationData(FName(*DstAniName));
}
