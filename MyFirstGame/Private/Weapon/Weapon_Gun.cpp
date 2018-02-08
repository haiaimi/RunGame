// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon_Gun.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Bullet.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AWeapon_Gun::AWeapon_Gun(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GunMesh = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("GunMseh"));
	BeamEmitter = CreateDefaultSubobject<UParticleSystem>(TEXT("BeamParticle"));
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
void AWeapon_Gun::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
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


void AWeapon_Gun::Fire(FVector ShootDir)
{
	FTransform SpawnTrans(FRotator::ZeroRotator, GetFireLocation());
	
	//第一种方法
	ABullet* SpawnBullet = GetWorld()->SpawnActorDeferred<ABullet>(ProjectileWeapon, SpawnTrans, this);
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

	//第二种方法
	/*ABullet* SpawnBullet = Cast<ABullet>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ProjectileWeapon, SpawnTrans));
	if (SpawnBullet)
	{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, "You have Shoot!");
	SpawnBullet->Instigator = this;
	SpawnBullet->InitBulletVelocity(ShootDir);

	UGameplayStatics::FinishSpawningActor(SpawnBullet, SpawnTrans);
	}*/
}
