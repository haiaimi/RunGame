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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStopGame);

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

	/**分数奖励的蓝图对象*/
	TSubclassOf<ABonus> Bonus_Score;

	/**无障碍模式奖励对象*/
	TSubclassOf<ABonus> Bonus_NoObstacle;

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

	/**是否在停止集合的动画中*/
	uint8 IsInStopToAllAnim : 1;

	int32 SpawnNoObsBonusParam;

	uint32 IsInPause : 1;

	/**游戏是否已结束*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint32 bIsGameEnd : 1;

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

	/**玩家移动距离*/
	//float PlayerMoveDistance = 0.f;

	FTimerHandle NoObstacleTime;

	UPROPERTY(BlueprintAssignable)
	FStopGame StopGameDelegate;

public:
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)override;

	virtual void Destroyed()override;

	virtual void BeginPlay()override;

protected:
	virtual void SetupInputComponent()override;

	virtual void PostInitializeComponents()override;

public:
	/**用于初始化平台*/
	void InitPlatforms();

	/**重新开始游戏*/
	UFUNCTION(BlueprintCallable)
	void RestartGame();

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
	void SpawnBonus_Score(ARunPlatform* AttachedPlatform);

	void SpawnBonus_NoObstacle(ARunPlatform* AttachedPlatform);

	void ChangeWeaponType(EWeaponType::Type WeaponType);

	/**随机生成飞行障碍*/
	void RandomSpawnFlyObstacle();

	void AddMaxSpawnObstacles();

	UFUNCTION(BlueprintCallable)
	void TogglePauseStat();

	UFUNCTION(BlueprintCallable)
	void QuitGame();

	/**所有平台集合
	  *@Param LastTime 无障碍模式持续的时间	
	  */
	void StartToAll(int32 LastTime);

	void StartToAllTest();

	/**进入停止集合的状态，但是不会执行实际操作*/
	void ToStopToAllState();

	/**停止集合，停止无障碍模式*/
	void StopToAll();

	void StopToAllAnimEnd();

	void NewSpawnedPlatformToAll(ARunPlatform* NewPlatformRef);
};
