// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldActors/RunPlatform.h"
#include "RunPlatform_Physic.generated.h"

/**
 * 
 */
UCLASS()
class MYFIRSTGAME_API ARunPlatform_Physic : public ARunPlatform
{
	GENERATED_UCLASS_BODY()

public:
	/**ƽ̨����������*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* AttachedMesh;

	/**����Լ�����*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Platform")
	class UPhysicsConstraintComponent* ConstraintComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Platform")
	class UParticleSystem* LinkParticle;

	class UParticleSystemComponent* SpawnedParticle;
	/**ƽ̨�����ں�Լ�������ӵĲ��*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Platform")
	FName LinkSocket;

	/**�Ƿ���ƽ̨����״̬*/
	uint8 IsToAll : 1;

public:
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)override;

	UFUNCTION()
	virtual void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	virtual void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)override;
	
	virtual void StartDestroy()override;

	virtual void MoveTick(float DeltaTime)override;

	virtual void MoveToAllFun(const FVector DeltaDistance)override;

protected:
	virtual void PostInitializeComponents()override;

};
