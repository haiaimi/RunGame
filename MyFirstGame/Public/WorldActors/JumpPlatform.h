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
	//下面的函数不需要调用父类
	UFUNCTION()
	virtual void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)override;

	UFUNCTION()
	virtual void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)override;
	
public:
	/**用于该平台下坠销毁*/
	FTimerHandle ToFall;

	/**该平台生成时的定向*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	FRotator SpawnRotation;

	class UCurveFloat* MoveCurve;

	/**该平台是否移动*/
	uint32 bCanMove : 1;

	float MoveCurveTime;

	/**用于平台移动差异化*/
	float MoveStartTime;
};
