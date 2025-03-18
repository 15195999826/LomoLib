// Fill out your copyright notice in the Description page of Project Settings.


#include "RapidUI/ButtonGroup/RapidButtonGroup.h"

#include "Components/PanelWidget.h"
#include "RapidUI/RapidIntPayloadSwitchBtn.h"

void URapidButtonGroup::BtnGroupNativePreConstruct(UPanelWidget* InButtonContainer)
{
	ButtonContainer = InButtonContainer;
	
	for (int32 i = 0; i < ButtonContainer->GetChildrenCount(); ++i)
	{
		URapidIntPayloadSwitchBtn* Btn = Cast<URapidIntPayloadSwitchBtn>(ButtonContainer->GetChildAt(i));
		if (Btn)
		{
			Btn->OnGroupButtonActive.AddDynamic(this, &URapidButtonGroup::OnButtonActive);
		}
	}
}

void URapidButtonGroup::ClearGroup()
{
	// 移除所有监听
	for (int32 i = 0; i < ButtonContainer->GetChildrenCount(); ++i)
	{
		URapidIntPayloadSwitchBtn* Btn = Cast<URapidIntPayloadSwitchBtn>(ButtonContainer->GetChildAt(i));
		if (Btn)
		{
			Btn->OnGroupButtonActive.RemoveAll(this);
		}
	}
	ButtonContainer->ClearChildren();
}

void URapidButtonGroup::PcgButtonGroup(TSubclassOf<URapidIntPayloadSwitchBtn> BtnClass, TArray<FRapidGroupBtnConfig> InMenuConfig)
{
	ClearGroup();
	
	for (const FRapidGroupBtnConfig& BtnConfig : InMenuConfig)
	{
		AppendPcgButton(BtnClass, BtnConfig);
	}
}

URapidIntPayloadSwitchBtn* URapidButtonGroup::AppendPcgButton(TSubclassOf<URapidIntPayloadSwitchBtn> BtnClass, const FRapidGroupBtnConfig& InBtnConfig)
{
	URapidIntPayloadSwitchBtn* Btn = CreateWidget<URapidIntPayloadSwitchBtn>(this, BtnClass);
	Btn->ProgramBindPayload(InBtnConfig.IntPayload, InBtnConfig.CustomViewFlag);
	ButtonContainer->AddChild(Btn);
	CustomProgramBtnStyle(Btn);
	Btn->OnGroupButtonActive.AddDynamic(this, &URapidButtonGroup::OnButtonActive);

	return Btn;
}

void URapidButtonGroup::DeActiveAll()
{
	for (int32 i = 0; i < ButtonContainer->GetChildrenCount(); ++i)
	{
		URapidIntPayloadSwitchBtn* Btn = Cast<URapidIntPayloadSwitchBtn>(ButtonContainer->GetChildAt(i));
		if (Btn)
		{
			Btn->NativeSetActive(false);
		}
	}
}

void URapidButtonGroup::SetDefaultActiveButton(int32 InIndex)
{
	URapidIntPayloadSwitchBtn* Btn = Cast<URapidIntPayloadSwitchBtn>(ButtonContainer->GetChildAt(InIndex));
	if (Btn)
	{
		Btn->NativeSetActive(true);
	}
}

void URapidButtonGroup::OnButtonActive(URapidIntPayloadSwitchBtn* InGroupButton, int32 InIntPayload, bool IsActive)
{
	if (!IsActive)
	{
		return;
	}
	
	if (ActiveButton.IsValid())
	{
		ActiveButton->NativeSetActive(false);
	}

	ActiveButton = InGroupButton;

	OnChangeActiveButton.Broadcast(InIntPayload);
}