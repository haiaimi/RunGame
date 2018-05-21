// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyFirstGame.h"
#include "Bullet.generated.h"

UCLASS(Abstract, Blueprintable)
class MYFIRSTGAME_API ABullet : public AActor
{
	GENERATED_UCLASS_BODY()
	
public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
	class UProjectileMovementComponent* ProjectileComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BulletMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
	class USphereComponent* BulletCollision;

	TWeakObjectPtr<AController> OwnerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet")
	TEnumAsByte<EWeaponType::Type> CurWeaponType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bullet")
	float LiveTime;

	/**发射该子弹的枪*/
	class AWeapon_Gun* OwnerWeapon;

	class UParticleSystemComponent* SpawnedParticle;

protected:
	// Called when the game starts or when spawned
	virtual void PostInitializeComponents() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnImpact(const FHitResult &result);

	void InitBulletVelocity(const FVector& ShootDir);

	/**把粒子的源点依附到平台上*/
	void ChangeParticleSourceToPlatform(FVector SourcePoint);
	
};
