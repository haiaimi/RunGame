// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldActors/RunPlatform.h"
#include "RunPlatform_Shoot.generated.h"

class ABoomActor;
/**
 * 这是通过射击才会旋转的平台
 */
UCLASS()
class MYFIRSTGAME_API ARunPlatform_Shoot : public ARunPlatform
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Platform")
	TSubclassOf<ABoomActor> Trigger;

	/**指向已生成的爆炸物*/
	ABoomActor* AimTrigger;

	/**生成的主动爆炸物*/
	ABoomActor* InitiativeBoom;

protected:
	virtual void PostInitializeComponents()override;
	
public:
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)override;

	virtual void MoveToAllTick(float DeltaTime)override;

	virtual void MoveToAllFun(const FVector DeltaDistance)override;
};
