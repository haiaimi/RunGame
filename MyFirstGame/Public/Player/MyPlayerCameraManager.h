// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "MyFirstGameCharacter.h"
#include "MyPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class MYFIRSTGAME_API AMyPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_UCLASS_BODY()

public:
	float FirstViewFOV;

	uint32 bUIGaussian : 1;

protected:
	virtual void UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime)override;
	
	virtual void PostInitializeComponents()override;

public:
	virtual void BeginPlay()override;

	/**����UI����ģ��*/
	UFUNCTION(BlueprintCallable)
	void StartGaussianUI();

	UFUNCTION(BlueprintCallable)
	void StopGaussianUI();
};
