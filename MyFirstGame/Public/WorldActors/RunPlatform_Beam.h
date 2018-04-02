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
	/**���ں�����ǹ���ӵ�����Ч��*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Platform")
	class UParticleSystem* AttachmentParticle;
	
	/*���ڼ�����ӵ���ײ��*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Platform")
	class UBoxComponent* AttachmentTrigger;

	class UParticleSystemComponent* ToParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Platform")
	FName AttachSocket;

	/**Ŀ��ǹ*/
	class AWeapon_Gun* TargetGun;

	/**ƽ̨���Ƿ���Ҫ��������*/
	uint8 UpdateBeam : 1;

	/**ָ�����ɵ���������*/
	class UParticleSystemComponent* BeamParticle;

	class UCurveFloat* MoveCurve;

	uint8 IsInMove : 1;

	/**�Ƿ������ƶ�*/
	uint8 IsMoveUp : 1;
	
	/**�ƶ��ķ���*/
	FVector MoveDir;

	/**��һ��ƽ̨��������ƽ̨�����λ��*/
	FVector NextPlatToCur;

	/**ƽ̨�ڱ��紥��ֹͣ�ƶ�ʱ��λ��*/
	FVector StopLocation;

	/**�洢SpawnLocation*/
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
	/**�����ӵ�������ײ�����*/
	UFUNCTION()
	void AttachBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	/**����������ڵ�ƽ̨*/
	void MoveToPlayerPlat();

	void DeActiveBeam();

	virtual void StartDestroy()override;

	virtual void MoveToAllFun(const FVector DeltaDistance)override;

	virtual void StopToAllFun(const FVector DeltaDistance)override;
};
