// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

class ARunPlatform;
class ABonus;
/**
 * 1���Զ�����ƽ̨
 */
UCLASS()
class MYFIRSTGAME_API AMyPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

public:
	ARunPlatform* CurPlatform;

	/***/
	ARunPlatform* TempPlatform;
		
	/**Ҫ���ɵ�ƽ̨*/
	TSubclassOf<ARunPlatform> SpawnPlatform;

	/**��Ҫ������ƽ̨*/
	TSubclassOf<ARunPlatform> SpawnPlatform_Shoot;

	/*������������ͼ����*/
	TSubclassOf<ABonus> Bonus_Score;

	/**��ŵ�ǰ����ƽ̨������*/
	TArray<ARunPlatform*> PlatformArray;

	/**���ɵ�ƽ̨�еľ��Է�����Ϣ*/
	uint8 AbsoluteDir;

public:
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction);

protected:
	virtual void SetupInputComponent()override;

	virtual void PostInitializeComponents()override;

public:
	/**�����Զ�����ƽ̨,���Ǿ����������ܴ�����ƽ̨*/
	FTransform GetSpawnTransf_Shoot(ARunPlatform* PrePlatform);

	/**�����ϸ�һ��ƽ̨�������������ģ���������ƽ̨��λ��*/
	FTransform GetRandomSpawnTransf(ARunPlatform* PrePlatform);

	/**�ڵ�ǰƽ̨������Bonus*/
	void SpawnBonus_Score(ARunPlatform* CurPlatform);

};
