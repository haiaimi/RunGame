// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyFirstGame.h"
#include "MyPlayerController.generated.h"

class ARunPlatform_Beam;
class ARunPlatform;
class ABonus;
class AFlyObstacle;
/**
 * 1���Զ�����ƽ̨
 * 2�����ɷ����ϰ�
 */
UCLASS()
class MYFIRSTGAME_API AMyPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

public:
	ARunPlatform* CurPlatform;

	/***/
	ARunPlatform* TempPlatform;
	
	/**��ǰ���ӵ�����ƽ̨*/
	ARunPlatform_Beam* CurConnectedPlat;

	/**Ҫ���ɵ�ƽ̨*/
	TSubclassOf<ARunPlatform> SpawnPlatform;

	/**��Ҫ������ƽ̨*/
	TSubclassOf<ARunPlatform> SpawnPlatform_Shoot;

	TSubclassOf<ARunPlatform> SpawnPlatform_Beam;

	TSubclassOf<ARunPlatform> SpawnPlatform_Physic;

	TSubclassOf<AFlyObstacle> SpawnFlyObstacle;

	/*������������ͼ����*/
	TSubclassOf<ABonus> Bonus_Score;

	/**��ŵ�ǰ����ƽ̨������*/
	TArray<ARunPlatform*> PlatformArray;

	/**��ŵ�ǰ������ϰ������飬��Shootƽ̨�ֲ�*/
	TArray<AFlyObstacle*> FlyObstacleArray;

	/**���ɵ�ƽ̨�еľ��Է�����Ϣ*/
	TEnumAsByte<EPlatformDirection::Type> AbsoluteDir;
	
	TEnumAsByte<EWeaponType::Type> CurrentWeaponType;

	/**��ǰ����Ƿ�������ƽ̨*/
	uint8 InConnectedToPlat : 1;

	/**�������������ϰ����ɵ���С���ƽ̨��*/
	int32 FlyObstacleSpawnInterval;

	/**ÿ��ƽ�������ķ����ϰ�*/
	int32 MaxFlyObstacles = 3;

	int32 CurFlyObstacles;

public:
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction);

	virtual void Destroyed()override;

	virtual void BeginPlay()override;

protected:
	virtual void SetupInputComponent()override;

	virtual void PostInitializeComponents()override;

public:
	/**�������ƽ̨�ĺ�����ָ�����ɵĸ���*/
	void RandomSpawnPlatform(int32 SpawnNum);

	/**�����Զ�����ƽ̨,���Ǿ����������ܴ�����ƽ̨*/
	FTransform GetSpawnTransf_Shoot(ARunPlatform* PrePlatform);

	/**�����ϸ�һ��ƽ̨�������������ģ���������ƽ̨��λ��*/
	FTransform GetRandomSpawnTransf(ARunPlatform* PrePlatform);

	/**��ȡ��������ƽ̨��λ��*/
	FTransform GetSpawnTransf_Beam(ARunPlatform* PrePlatform);

	/**��ȡ��������ƽ̨��λ��*/
	FTransform GetSpawnTransf_Physic(ARunPlatform* PrePlatform);

	/**�ڵ�ǰƽ̨������Bonus*/
	void SpawnBonus_Score(ARunPlatform* CurPlatform);

	void ChangeWeaponType(EWeaponType::Type WeaponType);

	/**������ɷ����ϰ�*/
	void RandomSpawnFlyObstacle();
};