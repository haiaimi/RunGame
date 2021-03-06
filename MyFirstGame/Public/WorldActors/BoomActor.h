﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoomActor.generated.h"

UCLASS()
class MYFIRSTGAME_API ABoomActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoomActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boom")
	class URadialForceComponent* RadialForce;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boom", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* WhatToBoom;

	UPROPERTY(EditDefaultsOnly, Category = "Boom")
	class UParticleSystem* BoomEmitter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boom")
	class USphereComponent* QueryChar;

	FTimerHandle SpawnParticle;

	//该定时器用来定时爆炸力作用时间
	FTimerHandle ForceTime;

	//该爆炸物是否可以爆炸
	uint8 CanBoom : 1;

	//是否已经爆炸
	uint8 IsBoom : 1;

	//是否是主动触发爆炸
	bool InitiativeToBoom = false;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	void  DestroyActor();

	void Boom();

	void StopForce();

	/**开始物理模拟*/
	void StartSimulatePhysic();

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

};
