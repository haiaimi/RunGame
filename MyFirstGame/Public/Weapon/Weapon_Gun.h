// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyFirstGame.h"
#include "Weapon_Gun.generated.h"

USTRUCT()
struct FWeaponData
{
	GENERATED_USTRUCT_BODY()

	/**所能携带最大子弹数量*/
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 MaxAmmo;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 AmmoPerClip;

	/**初始时子弹数量*/
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 InitialAmmo;

	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	int8 ShootSpeed;

	/**此枪的类型*/
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	TEnumAsByte<EWeaponType::Type> WeaponType;

	FWeaponData()
	{
		MaxAmmo = 300;
		AmmoPerClip = 30;
		InitialAmmo = 60;
		ShootSpeed = 10;
		WeaponType = EWeaponType::Weapon_Projectile;
	}
};

UCLASS(Abstract, Blueprintable)
class MYFIRSTGAME_API AWeapon_Gun : public AActor
{
	GENERATED_UCLASS_BODY()
	
public:	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GunData", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* GunMesh;

	UPROPERTY(EditDefaultsOnly, Category = "GunData")
	FWeaponData WeaponData;

	/**发射子弹所对相应的点（枪的骨骼上）*/
	UPROPERTY(EditDefaultsOnly, Category = "GunData")
	FName FirePoint;

	UPROPERTY(EditDefaultsOnly, Category = "GunData")
	class UParticleSystem* FireEmitter;

	/**闪电粒子*/
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GunData")
	class UParticleSystem* BeamEmitter;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<class ABullet> ProjectileWeapon;

protected:
	virtual void PostInitializeComponents()override;

public:	
	// Called every frame
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	FORCEINLINE UParticleSystem* GetFireParticle() { return FireEmitter; }

	FVector GetFireLocation();

	FTransform GetFireTransform();

	/**把开火函数放在枪类中*/
	virtual void Fire(FVector FireDir);
};
