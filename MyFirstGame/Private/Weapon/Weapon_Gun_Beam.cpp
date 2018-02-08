// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon_Gun_Beam.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Bullet.h"
#include "Engine/World.h"


AWeapon_Gun_Beam::AWeapon_Gun_Beam(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	//BeamEmitter = ObjectInitializer.CreateDefaultSubobject<UParticleSystem>(this, TEXT("BeamParticle"));
}

void AWeapon_Gun_Beam::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

}

void AWeapon_Gun_Beam::Fire(FVector ShootDir)
{
	FTransform SpawnTrans(FRotator::ZeroRotator, GetFireLocation());

	ABullet* SpawnBullet = GetWorld()->SpawnActorDeferred<ABullet>(ProjectileWeapon, SpawnTrans);
	if (SpawnBullet)
	{
		SpawnBullet->InitBulletVelocity(ShootDir);

		UGameplayStatics::FinishSpawningActor(SpawnBullet, SpawnTrans);
	}

	//释放开火特效
	if (GetFireParticle())
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), GetFireParticle(), GetFireLocation());
	}
}