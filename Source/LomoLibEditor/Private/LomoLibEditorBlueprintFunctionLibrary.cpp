// Fill out your copyright notice in the Description page of Project Settings.


#include "LomoLibEditorBlueprintFunctionLibrary.h"

#include "PackageTools.h"
#include "Animation/ProgramAnimationDataAsset.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"

EProgramAnimationAssetCreateResult ULomoLibEditorBlueprintFunctionLibrary::CreateProgramAnimationDataAsset(
	const FProgramAnimationData& AnimationData,
	const FString& AssetPath,
	bool bOverwrite)
{
	// 验证资产路径格式
	if (!AssetPath.StartsWith(TEXT("/Game/")) && !AssetPath.Contains(TEXT("/")))
	{
		UE_LOG(LogTemp, Error, TEXT("CreateProgramAnimationDataAsset: 资产路径格式无效: %s"), *AssetPath);
		return EProgramAnimationAssetCreateResult::Failed;
	}

	// 解析包路径和资产名称
	FString PackagePath;
	FString AssetName;
	
	if (AssetPath.Contains(TEXT("/")))
	{
		FString Left, Right;
		AssetPath.Split(TEXT("/"), &Left, &Right, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		PackagePath = Left;
		AssetName = Right;
	}
	else
	{
		PackagePath = TEXT("/Game");
		AssetName = AssetPath;
	}

	// 确保资产名称有效
	if (AssetName.IsEmpty())
	{
		AssetName = TEXT("NewProgramAnimationDataAsset");
	}

	// 创建唯一的包名
	FString PackageName = PackagePath + TEXT("/") + AssetName;
	PackageName = UPackageTools::SanitizePackageName(PackageName);
	
	// 检查资产是否已存在
	bool bAssetExists = false;
	UObject* ExistingAsset = LoadObject<UObject>(nullptr, *PackageName);
	if (ExistingAsset)
	{
		bAssetExists = true;
		if (!bOverwrite)
		{
			UE_LOG(LogTemp, Warning, TEXT("CreateProgramAnimationDataAsset: 资产已存在且不允许覆盖: %s"), *PackageName);
			return EProgramAnimationAssetCreateResult::Failed;
		}
		UE_LOG(LogTemp, Warning, TEXT("CreateProgramAnimationDataAsset: 资产已存在，将进行覆盖: %s"), *PackageName);
	}

	// 创建新的包
	UPackage* Package = CreatePackage(*PackageName);
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateProgramAnimationDataAsset: 无法创建包: %s"), *PackageName);
		return EProgramAnimationAssetCreateResult::Failed;
	}

	// 创建新的UProgramAnimationDataAsset实例
	UProgramAnimationDataAsset* NewAsset = NewObject<UProgramAnimationDataAsset>(
		Package, 
		UProgramAnimationDataAsset::StaticClass(), 
		*AssetName, 
		RF_Public | RF_Standalone
	);

	if (!NewAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateProgramAnimationDataAsset: 无法创建资产对象"));
		return EProgramAnimationAssetCreateResult::Failed;
	}

	// 设置动画数据
	NewAsset->AnimationData = AnimationData;

	// 标记包为脏状态
	Package->MarkPackageDirty();

	// 通知资产注册表
	FAssetRegistryModule::AssetCreated(NewAsset);

	// 保存包
	FString const PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	
	// 使用UE5的FSavePackageArgs
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;
	bool bSaved = UPackage::SavePackage(Package, NewAsset, *PackageFileName, SaveArgs);

	if (bSaved)
	{
		if (bAssetExists)
		{
			UE_LOG(LogTemp, Log, TEXT("CreateProgramAnimationDataAsset: 成功覆盖并保存资产: %s"), *PackageFileName);
			return EProgramAnimationAssetCreateResult::Overwritten;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("CreateProgramAnimationDataAsset: 成功创建并保存资产: %s"), *PackageFileName);
			return EProgramAnimationAssetCreateResult::Success;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("CreateProgramAnimationDataAsset: 保存资产失败: %s"), *PackageFileName);
	return EProgramAnimationAssetCreateResult::Failed;
}
