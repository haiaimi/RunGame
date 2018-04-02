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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStopGame);

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

	/**������������ͼ����*/
	TSubclassOf<ABonus> Bonus_Score;

	/**���ϰ�ģʽ��������*/
	TSubclassOf<ABonus> Bonus_NoObstacle;

	/**��ŵ�ǰ����ƽ̨������*/
	UPROPERTY()
	TArray<ARunPlatform*> PlatformArray;

	/**��ŵ�ǰ������ϰ������飬��Shootƽ̨�ֲ�*/
	UPROPERTY()
	TArray<AFlyObstacle*> FlyObstacleArray;

	/**���ɵ�ƽ̨�еľ��Է�����Ϣ*/
	TEnumAsByte<EPlatformDirection::Type> AbsoluteDir;
	
	TEnumAsByte<EWeaponType::Type> CurrentWeaponType;

	/**��ǰ����Ƿ�������ƽ̨*/
	uint8 InConnectedToPlat : 1;

	/**ƽ̨�Ƿ��ھۼ�״̬*/
	uint8 IsToAll : 1;

	/**�Ƿ���ֹͣ���ϵĶ�����*/
	uint8 IsInStopToAllAnim : 1;

	int32 SpawnNoObsBonusParam;

	uint32 IsInPause : 1;

	/**��Ϸ�Ƿ��ѽ���*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint32 bIsGameEnd : 1;

	/**�������������ϰ����ɵ���С���ƽ̨��*/
	int32 FlyObstacleSpawnInterval;

	/**ÿ��ƽ�������ķ����ϰ������Ų�����ߣ����ɵķ����ϰ�Ҳ���*/
	int32 MaxFlyObstacles;

	/**��ǰ�Ѿ����ɵ�Shootƽ̨��Ҫ��������������ɵ�FlyObstacle������*/
	int32 CurSpawnedShootPlats;

	int32 CurFlyObstacles;

	int32 HasSpawnedPlatNum = 0;

	int32 HasSpawnedBeamPlatNum = 0;

	FVector DeltaLocToPrePlat;

	/**����ƶ�����*/
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
	/**���ڳ�ʼ��ƽ̨*/
	void InitPlatforms();

	/**���¿�ʼ��Ϸ*/
	UFUNCTION(BlueprintCallable)
	void RestartGame();

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
	void SpawnBonus_Score(ARunPlatform* AttachedPlatform);

	void SpawnBonus_NoObstacle(ARunPlatform* AttachedPlatform);

	void ChangeWeaponType(EWeaponType::Type WeaponType);

	/**������ɷ����ϰ�*/
	void RandomSpawnFlyObstacle();

	void AddMaxSpawnObstacles();

	UFUNCTION(BlueprintCallable)
	void TogglePauseStat();

	UFUNCTION(BlueprintCallable)
	void QuitGame();

	/**����ƽ̨����
	  *@Param LastTime ���ϰ�ģʽ������ʱ��	
	  */
	void StartToAll(int32 LastTime);

	void StartToAllTest();

	/**����ֹͣ���ϵ�״̬�����ǲ���ִ��ʵ�ʲ���*/
	void ToStopToAllState();

	/**ֹͣ���ϣ�ֹͣ���ϰ�ģʽ*/
	void StopToAll();

	void StopToAllAnimEnd();

	void NewSpawnedPlatformToAll(ARunPlatform* NewPlatformRef);
};
