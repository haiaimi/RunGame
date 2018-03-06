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
	
	/**飞行障碍所追踪的目标人物*/
	class AMyFirstGameCharacter* AimCharacter;

	/**当前障碍物飞行的方向，可变的*/
	FVector FlyDir;

	/**飞行的目标位置*/
	FVector FlyDst;

	FVector HalfObstacleSize;

	/**平台位置*/
	//TArray<FVector> PlatformsLocation;

	/**定时检查角度的定时器*/
	FTimerHandle QueryAngler;

	/**删除对象的定时器*/
	FTimerHandle DestroyTimer;

	/**飞行障碍物开始时的速度*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Obstacle")
	float StartSpeed;

	/**飞行障碍物的加速度*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Obstacle")
	float AccelerateSpeed;

	/**减速到0的加速度，为负*/
	float ToStopAccelerate;

	/**当前速度*/
	float CurSpeed;

	/**记录该障碍到人物的最高的速度*/
	float ToCharMaxSpeed;

	/**是否已经超过目标点，超过就准备减速返回*/
	uint8 IsOver : 1;

	/**这是在还没有激活障碍物移动条件下，强制移动*/
	uint8 ForceActive : 1;

	/**障碍物静止的时间*/
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
	/**该函数是判断飞行障碍物和玩家的角度*/
	void QueryIsOverSubAngle();

	void StartDestroy();

	void DestroyActor();

	/**该函数是为了在障碍物寻找合适的停止位置，以免影响人物移动*/
	void SelectSuitStopAccelerate(FVector MoveDir, float CurSpeed, float MoveDistance);

	/**根据给出的速度，以及加速度求出导停止的距离*/
	float ComputeDistanceToStop(float CurSpeed, float Accelerate);

	/**根据给出的移动距离，初速度，计算移动到零时的加速度*/
	float ComputeAccelerateToStop(float CurSpeed, float MoveDistance);
};
