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
	/**平台的依附物体*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* AttachedMesh;

	/**物理约束组件*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Platform")
	class UPhysicsConstraintComponent* ConstraintComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Platform")
	class UParticleSystem* LinkParticle;

	class UParticleSystemComponent* SpawnedParticle;
	/**平台上用于和约束点连接的插槽*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Platform")
	FName LinkSocket;

	/**是否在平台集合状态*/
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
