// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldActors/RunPlatform.h"
#include "RunPlatform_Beam.generated.h"

/**
 * 
 */
UCLASS()
class MYFIRSTGAME_API ARunPlatform_Beam : public ARunPlatform
{
	GENERATED_UCLASS_BODY()
	
public:
	/**用于和闪电枪连接的粒子效果*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Platform")
	class UParticleSystem* AttachmentParticle;
	
	/*用于检测连接的碰撞体*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Platform")
	class UBoxComponent* AttachmentTrigger;

	class UParticleSystemComponent* ToParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Platform")
	FName AttachSocket;

	/**目标枪*/
	class AWeapon_Gun* TargetGun;

	/**平台上是否需要更新闪电*/
	uint8 UpdateBeam : 1;

	/**指向生成的闪电粒子*/
	class UParticleSystemComponent* BeamParticle;

	class UCurveFloat* MoveCurve;

	uint8 IsInMove : 1;

	/**是否向上移动*/
	uint8 IsMoveUp : 1;
	
	/**移动的方向*/
	FVector MoveDir;

	/**下一个平台到该闪电平台的相对位置*/
	FVector NextPlatToCur;

	/**平台在被电触发停止移动时的位置*/
	FVector StopLocation;

	/**存储SpawnLocation*/
	FVector TempSpawnLocation;     

	float MoveCycle = 0;

public:
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)override;

	virtual void BeginPlay()override;

	virtual void MoveTick(float DeltaTime)override;

	UFUNCTION()
	virtual void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)override;

protected:
	virtual void PostInitializeComponents()override;

public:
	/**闪电子弹进入碰撞检测体*/
	UFUNCTION()
	void AttachBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	/**移向玩家所在的平台*/
	void MoveToPlayerPlat();

	void DeActiveBeam();

	virtual void StartDestroy()override;

	virtual void MoveToAllFun(const FVector DeltaDistance)override;

	virtual void StopToAllFun(const FVector DeltaDistance)override;
};
