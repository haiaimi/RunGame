// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon_Gun.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystem.h"


// Sets default values
AWeapon_Gun::AWeapon_Gun(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GunMesh = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("GunMseh"));
	FireEmitter = ObjectInitializer.CreateDefaultSubobject<UParticleSystem>(this, TEXT("FireEmitter"));
	if (GunMesh)
	{
		GunMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		GunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RootComponent = GunMesh;
	}
}

// Called when the game starts or when spawned
void AWeapon_Gun::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
}

// Called every frame
void AWeapon_Gun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector AWeapon_Gun::GetFireLocation()
{
	if (GunMesh)
	{
		FVector FireLocation = GunMesh->GetSocketLocation(FirePoint);
		return FireLocation;
	}
	return FVector::ZeroVector;
}

FTransform AWeapon_Gun::GetFireTransform()
{
	if (GunMesh)
	{
		return GunMesh->GetSocketTransform(FirePoint);
	}
	return FTransform(NoInit);
}

