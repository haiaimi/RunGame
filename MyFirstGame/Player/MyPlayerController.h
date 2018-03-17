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
 * 1、自动生成平台
 * 2、生成飞行障碍
 */
UCLASS()
class MYFIRSTGAME_API AMyPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

public:
	ARunPlatform* CurPlatform;

	/***/
	ARunPlatform* TempPlatform;
	
	/**当前连接的闪电平台*/
	ARunPlatform_Beam* CurConnectedPlat;

	/**要生成的平台*/
	TSubclassOf<ARunPlatform> SpawnPlatform;

	/**需要触发的平台*/
	TSubclassOf<ARunPlatform> SpawnPlatform_Shoot;

	TSubclassOf<ARunPlatform> SpawnPlatform_Beam;

	TSubclassOf<ARunPlatform> SpawnPlatform_Physic;

	TSubclassOf<AFlyObstacle> SpawnFlyObstacle;

	/*分数奖励的蓝图对象*/
	TSubclassOf<ABonus> Bonus_Score;

	/**存放当前所有平台的数组*/
	UPROPERTY()
	TArray<ARunPlatform*> PlatformArray;

	/**存放当前层飞行障碍的数组，以Shoot平台分层*/
	UPROPERTY()
	TArray<AFlyObstacle*> FlyObstacleArray;

	/**生成的平台中的绝对方向信息*/
	TEnumAsByte<EPlatformDirection::Type> AbsoluteDir;
	
	TEnumAsByte<EWeaponType::Type> CurrentWeaponType;

	/**当前玩家是否连接着平台*/
	uint8 InConnectedToPlat : 1;

	/**平台是否在聚集状态*/
	uint8 IsToAll : 1;

	uint32 IsInPause : 1;

	/**这是两个飞行障碍生成的最小间隔平台数*/
	int32 FlyObstacleSpawnInterval;

	/**每个平面上最多的飞行障碍，随着层数变高，生成的飞行障碍也变多*/
	int32 MaxFlyObstacles;

	/**当前已经生成的Shoot平台，要根据这个控制生成的FlyObstacle得数量*/
	int32 CurSpawnedShootPlats;

	int32 CurFlyObstacles;

	int32 HasSpawnedPlatNum = 0;

	int32 HasSpawnedBeamPlatNum = 0;

	FVector DeltaLocToPrePlat;

public:
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction);

	virtual void Destroyed()override;

	virtual void BeginPlay()override;

protected:
	virtual void SetupInputComponent()override;

	virtual void PostInitializeComponents()override;

public:
	/**随机生成平台的函数，指定生成的个数*/
	void RandomSpawnPlatform(int32 SpawnNum);

	/**用于自动生成平台,这是经过触发才能触发的平台*/
	FTransform GetSpawnTransf_Shoot(ARunPlatform* PrePlatform);

	/**根据上个一个平台（相对于数组里的）计算生成平台的位置*/
	FTransform GetRandomSpawnTransf(ARunPlatform* PrePlatform);

	/**获取生成闪电平台的位置*/
	FTransform GetSpawnTransf_Beam(ARunPlatform* PrePlatform);

	/**获取生成物理平台的位置*/
	FTransform GetSpawnTransf_Physic(ARunPlatform* PrePlatform);

	/**在当前平台上生成Bonus*/
	void SpawnBonus_Score(ARunPlatform* CurPlatform);

	void ChangeWeaponType(EWeaponType::Type WeaponType);

	/**随机生成飞行障碍*/
	void RandomSpawnFlyObstacle();

	void AddMaxSpawnObstacles();

	void TogglePauseStat();

	/**所有平台集合*/
	void StartToAll();

	/**停止集合，停止无障碍模式*/
	void StopToAll();

	void NewSpawnedPlatformToAll(ARunPlatform* NewPlatformRef);
};
