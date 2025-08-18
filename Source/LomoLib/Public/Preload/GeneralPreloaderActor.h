// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PreloadProvider.h"
#include "GameFramework/Actor.h"
#include "GeneralPreloaderActor.generated.h"

class APreloadActor;
class UNiagaraSystem;

// 声明日志
DECLARE_LOG_CATEGORY_EXTERN(LogGeneralPreloadProvider, Log, All);

UCLASS()
class LOMOLIB_API AGeneralPreloaderActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGeneralPreloaderActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY()
	FPreloadData PreloadData;

	UPROPERTY(BlueprintReadOnly)
	int TotalPreLoadCount;

	int PerFrameLoadCount = 10;
	int TempMatCursor = 0;
	int TempNiagaraCursor = 0;
	int TempStaticMeshCursor = 0;
	int LoadCount;

	int PostCount = 0;
	int CheckEndPassCount = 0;
	void CheckEnd();
	
public:
	virtual void StartPreload(const FPreloadData& InPreloadData);

protected:
	void LoadPerFrame();

	UFUNCTION(BlueprintImplementableEvent)
	APreloadActor* PreLoadMaterial(const TSoftObjectPtr<UMaterial>& Material);
	
	UFUNCTION(BlueprintImplementableEvent)
	APreloadActor* PreLoadNiagaraSystem(const TSoftObjectPtr<UNiagaraSystem>& NiagaraSystem);

	UFUNCTION(BlueprintImplementableEvent)
	APreloadActor* PreLoadStaticMesh(const TSoftObjectPtr<UStaticMesh>& StaticMesh);

	UFUNCTION(BlueprintImplementableEvent)
	void PostLoadedCount(int LoadedCount);

	UFUNCTION(BlueprintImplementableEvent)
	void OnLoadFinish();

	bool PIEAreCompiling() const;
};
