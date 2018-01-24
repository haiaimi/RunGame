// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

class ARunPlatform;
/**
 * 1、自动生成平台
 */
UCLASS()
class MYFIRSTGAME_API AMyPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

public:
	ARunPlatform* CurPlatform;

	/***/
	ARunPlatform* TempPlatform;
		
	/**要生成的平台*/
	TSubclassOf<ARunPlatform> SpawnPlatform;

	/**存放当前所有平台的数组*/
	TArray<ARunPlatform*> PlatformArray;

	/**生成的平台中的绝对方向信息*/
	uint8 AbsoluteDir;

public:
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction);

protected:
	virtual void SetupInputComponent()override;

	virtual void PostInitializeComponents()override;

public:
	/**用于自动生成平台*/
	void AutoSpawnPlatform();

	/**根据上个一个平台（相对于数组里的）计算生成平台的位置*/
	FTransform GetRandomSpawnTransf(ARunPlatform* PrePlatform);

};
