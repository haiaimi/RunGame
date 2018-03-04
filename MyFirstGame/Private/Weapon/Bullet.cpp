// Fill out your copyright notice in the Description page of Project Settings.

#include "Bullet.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "WorldActors/BoomActor.h"
#include "MyFirstGame.h"
#include "Classes/GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Weapon_Gun.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
ABullet::ABullet(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProjectileComponent = ObjectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("BulletProjectile"));
	BulletMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("BulletMesh"));
	BulletCollision = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("BulletCollision"));

	BulletCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);          //ֻ��������⣬����������ģ��
	BulletCollision->SetCollisionObjectType(COLLISION_PROJECTILE);          //���ø�Collision�Ķ�������
	BulletCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	BulletCollision->SetCollisionResponseToChannel(COLLISION_BOOMQUERY, ECollisionResponse::ECR_Ignore);  //���Ա�ը�����
	BulletCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECollisionResponse::ECR_Block);  
	
	BulletMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BulletMesh->SetCollisionObjectType(COLLISION_BOOMQUERY); 
	BulletMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RootComponent = BulletCollision;
	BulletMesh->SetupAttachment(RootComponent);
	BulletMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));
	RootComponent->SetWorldScale3D(FVector(0.1f, 0.1f, 0.1f));

	ProjectileComponent->InitialSpeed = 10000.f;
	ProjectileComponent->MaxSpeed = 10000.f;
	ProjectileComponent->UpdatedComponent = BulletCollision;
	ProjectileComponent->bRotationFollowsVelocity = true;
	ProjectileComponent->ProjectileGravityScale = 0.5f;

	//CurWeaponType = EWeaponType::Weapon_Projectile;
	PrimaryActorTick.TickGroup = TG_PrePhysics;     //������ģ��֮ǰ���е�����Ҫ��ִ��
}

// Called when the game starts or when spawned
void ABullet::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SetLifeSpan(LiveTime);
	ProjectileComponent->OnProjectileStop.AddDynamic(this, &ABullet::OnImpact);
	if(Instigator)
	{
		BulletCollision->MoveIgnoreActors.Add(Instigator);    //�����������
	}
	
	OwnerController = GetInstigatorController();

	AWeapon_Gun* Temp = Cast<AWeapon_Gun>(GetOwner());
	if (Temp != NULL)
	{
		CurWeaponType = Temp->WeaponData.WeaponType;
		OwnerWeapon = Temp;
		if (CurWeaponType == EWeaponType::Weapon_Beam && OwnerWeapon->BeamEmitter != NULL)
		{
			SpawnedParticle = UGameplayStatics::SpawnEmitterAttached(OwnerWeapon->BeamEmitter, BulletMesh, TEXT("ParticleSocket"));    //������������
			SpawnedParticle->SetWorldScale3D(FVector(5.f, 5.f, 5.f));
			SpawnedParticle->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
		}
	}
}

// Called every frame
void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (OwnerWeapon &&CurWeaponType == EWeaponType::Type::Weapon_Beam) //ֻ������ǹ�Ż����
	{
		SpawnedParticle->SetBeamTargetPoint(0, OwnerWeapon->GetFireLocation(), 0);
		//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, TEXT("Beam is in Update"));
	}
}

void ABullet::OnImpact(const FHitResult& HitResult)
{
	if (Cast<ABoomActor>(HitResult.GetActor()))
	{
	//	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, "Impact Boom!");
		ABoomActor* Aim = Cast<ABoomActor>(HitResult.GetActor());
		if(Aim->CanBoom)
			Aim->Boom();
	}
	if (HitResult.GetComponent()->IsSimulatingPhysics() && HitResult.GetActor() != this)
	{
		if (CurWeaponType == EWeaponType::Weapon_Projectile)
			HitResult.GetComponent()->AddImpulseAtLocation(GetVelocity() * 5, GetActorLocation());  //��ǰ��������ʩ��һ������
		if (CurWeaponType == EWeaponType::Weapon_Beam)
			HitResult.GetComponent()->AddImpulseAtLocation(GetVelocity() * 100, GetActorLocation());
	}

	Destroy();
}

void ABullet::InitBulletVelocity(const FVector& ShootDir)
{
	ProjectileComponent->Velocity = ProjectileComponent->InitialSpeed * ShootDir;
}

void ABullet::ChangeParticleSourceToPlatform(FVector SourcePoint)
{
	SpawnedParticle->SetBeamSourcePoint(0, SourcePoint, 0);
}