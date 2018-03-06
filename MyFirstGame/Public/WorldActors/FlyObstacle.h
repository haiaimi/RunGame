// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlyObstacle.generated.h"

UCLASS()
class MYFIRSTGAME_API AFlyObstacle : public AActor
{
	GENERATED_UCLASS_BODY()
	
public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Obstacle", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* FlyObstacleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Obstacle")
	class USceneComponent* SceneRoot;
	
	/**�����ϰ���׷�ٵ�Ŀ������*/
	class AMyFirstGameCharacter* AimCharacter;

	/**��ǰ�ϰ�����еķ��򣬿ɱ��*/
	FVector FlyDir;

	/**���е�Ŀ��λ��*/
	FVector FlyDst;

	FVector HalfObstacleSize;

	/**ƽ̨λ��*/
	//TArray<FVector> PlatformsLocation;

	/**��ʱ���ǶȵĶ�ʱ��*/
	FTimerHandle QueryAngler;

	/**ɾ������Ķ�ʱ��*/
	FTimerHandle DestroyTimer;

	/**�����ϰ��￪ʼʱ���ٶ�*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Obstacle")
	float StartSpeed;

	/**�����ϰ���ļ��ٶ�*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Obstacle")
	float AccelerateSpeed;

	/**���ٵ�0�ļ��ٶȣ�Ϊ��*/
	float ToStopAccelerate;

	/**��ǰ�ٶ�*/
	float CurSpeed;

	/**��¼���ϰ����������ߵ��ٶ�*/
	float ToCharMaxSpeed;

	/**�Ƿ��Ѿ�����Ŀ��㣬������׼�����ٷ���*/
	uint8 IsOver : 1;

	/**�����ڻ�û�м����ϰ����ƶ������£�ǿ���ƶ�*/
	uint8 ForceActive : 1;

	/**�ϰ��ﾲֹ��ʱ��*/
	float StopLengthTime;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents()override;

public:	
	// Called every frame
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	virtual void Destroyed()override;

public:
	/**�ú������жϷ����ϰ������ҵĽǶ�*/
	void QueryIsOverSubAngle();

	void StartDestroy();

	void DestroyActor();

	/**�ú�����Ϊ�����ϰ���Ѱ�Һ��ʵ�ֹͣλ�ã�����Ӱ�������ƶ�*/
	void SelectSuitStopAccelerate(FVector MoveDir, float CurSpeed, float MoveDistance);

	/**���ݸ������ٶȣ��Լ����ٶ������ֹͣ�ľ���*/
	float ComputeDistanceToStop(float CurSpeed, float Accelerate);

	/**���ݸ������ƶ����룬���ٶȣ������ƶ�����ʱ�ļ��ٶ�*/
	float ComputeAccelerateToStop(float CurSpeed, float MoveDistance);
};
