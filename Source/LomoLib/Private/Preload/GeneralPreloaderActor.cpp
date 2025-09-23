// Fill out your copyright notice in the Description page of Project Settings.


#include "Preload/GeneralPreloaderActor.h"

#if WITH_EDITOR
#include "AssetCompilingManager.h"
#include "ShaderCompiler.h"
#endif
// 声明日志
DEFINE_LOG_CATEGORY(LogGeneralPreloadProvider);

// Sets default values
AGeneralPreloaderActor::AGeneralPreloaderActor(): TotalPreLoadCount(0), LoadCount(0)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGeneralPreloaderActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGeneralPreloaderActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGeneralPreloaderActor::CheckEnd()
{
#if WITH_EDITOR
	if (!PIEAreCompiling())
	{
		CheckEndPassCount++;
	}
	else
	{
		CheckEndPassCount = 0;
	}

	// Todo: 通过20次检查， 就认为不再有后续的加载了
	if (CheckEndPassCount < 20)
	{
		FTimerHandle UnusedHandle;
		GetWorld()->GetTimerManager().SetTimer(UnusedHandle, this, &AGeneralPreloaderActor::CheckEnd, 0.1f, false);
		return;
	}
	
	OnLoadFinish();
#else
	// Todo: 
	return;
#endif
}

void AGeneralPreloaderActor::StartPreload(const FPreloadData& InPreloadData)
{
	PreloadData = InPreloadData;
	TotalPreLoadCount = PreloadData.Materials.Num() + PreloadData.NiagaraSystems.Num() + PreloadData.StaticMeshes.Num();
	PerFrameLoadCount = PreloadData.PerFrameLoadCount;

	if (TotalPreLoadCount <= 0)
	{
		UE_LOG(LogGeneralPreloadProvider, Log, TEXT("No preload data found!"));
		return;
	}
	
	// 每帧加载指定数量的资源
	LoadPerFrame();
}

void AGeneralPreloaderActor::LoadPerFrame()
{
#if WITH_EDITOR
	if (PIEAreCompiling())
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AGeneralPreloaderActor::LoadPerFrame);
		return;
	}

	if (TempStaticMeshCursor < PreloadData.StaticMeshes.Num())
	{
		for (int i = 0; i < PerFrameLoadCount; i++)
		{
			auto PreloadActor = PreLoadStaticMesh(PreloadData.StaticMeshes[TempStaticMeshCursor]);
			LoadCount++;
			TempStaticMeshCursor++;
			if (TempStaticMeshCursor >= PreloadData.StaticMeshes.Num())
			{
				break;
			}
		}
	}
	else if (TempMatCursor < PreloadData.Materials.Num())
	{
		for (int i = 0; i < PerFrameLoadCount; i++)
		{
			auto PreloadActor = PreLoadMaterial(PreloadData.Materials[TempMatCursor]);
			LoadCount++;
			TempMatCursor++;
			if (TempMatCursor >= PreloadData.Materials.Num())
			{
				break;
			}
		}
	}
	else if (TempNiagaraCursor < PreloadData.NiagaraSystems.Num())
	{
		for (int i = 0; i < PerFrameLoadCount; i++)
		{
			auto PreloadActor = PreLoadNiagaraSystem(PreloadData.NiagaraSystems[TempNiagaraCursor]);
			LoadCount++;
			TempNiagaraCursor++;
			if (TempNiagaraCursor >= PreloadData.NiagaraSystems.Num())
			{
				break;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadCount: %d, Totoal: %d"), LoadCount, TotalPreLoadCount);
		FTimerHandle UnusedHandle;
		GetWorld()->GetTimerManager().SetTimer(UnusedHandle, this, &AGeneralPreloaderActor::CheckEnd, 0.1f, false);
		return;
	}

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AGeneralPreloaderActor::LoadPerFrame);
#else
	// Todo: 打包后预加载一般跟项目强相关, 暂时不做处理, 未来有需要考虑暴露的接口的设计
	return;
#endif
}

bool AGeneralPreloaderActor::PIEAreCompiling() const
{
	bool AreAssetsCompiling =  false;
#if UE_EDITOR
	int AssetCompilingCount = FAssetCompilingManager::Get().GetNumRemainingAssets();
	UE_LOG(LogGeneralPreloadProvider, Log, TEXT("AssetCompilingCount: %d"), AssetCompilingCount);
	AreAssetsCompiling = AssetCompilingCount > 0 || (GShaderCompilingManager->GetNumPendingJobs() + GShaderCompilingManager->GetNumRemainingJobs()) > 0;
#endif
	return AreAssetsCompiling;
}

