// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldActors/RunPlatform.h"
#include "JumpPlatform.generated.h"

/**
 * 
 */
UCLASS()
class MYFIRSTGAME_API AJumpPlatform : public ARunPlatform
{
	GENERATED_UCLASS_BODY()
	
public:
	virtual void BeginPlay()override;

	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)override;

	virtual void PostInitializeComponents()override;

public:
	//����ĺ�������Ҫ���ø���
	UFUNCTION()
	virtual void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)override;

	UFUNCTION()
	virtual void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)override;
	
public:
	/**���ڸ�ƽ̨��׹����*/
	FTimerHandle ToFall;

	/**��ƽ̨����ʱ�Ķ���*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	FRotator SpawnRotation;

	class UCurveFloat* MoveCurve;

	/**��ƽ̨�Ƿ��ƶ�*/
	uint32 bCanMove : 1;

	float MoveCurveTime;

	/**����ƽ̨�ƶ����컯*/
	float MoveStartTime;
};
