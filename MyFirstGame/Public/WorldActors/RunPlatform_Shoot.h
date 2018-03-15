// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldActors/RunPlatform.h"
#include "RunPlatform_Shoot.generated.h"

class ABoomActor;
/**
 * ����ͨ������Ż���ת��ƽ̨
 */
UCLASS()
class MYFIRSTGAME_API ARunPlatform_Shoot : public ARunPlatform
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Platform")
	TSubclassOf<ABoomActor> Trigger;

	/**ָ�������ɵı�ը��*/
	ABoomActor* AimTrigger;

	/**���ɵ�������ը��*/
	ABoomActor* InitiativeBoom;

protected:
	virtual void PostInitializeComponents()override;
	
public:
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)override;

	virtual void MoveToAllTick(float DeltaTime)override;

	virtual void MoveToAllFun(const FVector DeltaDistance)override;
};
