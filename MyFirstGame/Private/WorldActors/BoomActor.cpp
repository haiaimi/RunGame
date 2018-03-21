// Fill out your copyright notice in the Description page of Project Settings.

#include "BoomActor.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/SphereComponent.h"
#include "MyFirstGameCharacter.h"
#include "MyFirstGame.h"


// Sets default values
ABoomActor::ABoomActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RadialForce = CreateDefaultSubobject<URadialForceComponent>("RadialForce");
	WhatToBoom = CreateDefaultSubobject<UStaticMeshComponent>("BoomMesh");
	BoomEmitter = CreateDefaultSubobject<UParticleSystem>("BoomEmitter");
	QueryChar = CreateDefaultSubobject<USphereComponent>("QueryCharacter");
	QueryChar->SetSphereRadius(250.f);
	QueryChar->SetCollisionObjectType(COLLISION_BOOMQUERY);    //设置检测体的碰撞类型
	QueryChar->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	QueryChar->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	QueryChar->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECollisionResponse::ECR_Ignore);    //忽略子弹碰撞检测
	QueryChar->SetCollisionEnabled(ECollisionEnabled::QueryOnly);      //只用于检测

	WhatToBoom->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	WhatToBoom->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECollisionResponse::ECR_Block);   //回应子弹碰撞检测
	RootComponent = WhatToBoom;

	QueryChar->SetupAttachment(WhatToBoom);
	WhatToBoom->SetRelativeLocation(FVector(0.f, 0.f, 0.f));  
	RadialForce->SetupAttachment(RootComponent);
	RadialForce->SetRelativeLocation(FVector::ZeroVector);
	RadialForce->SetActive(false);                   //默认设置爆炸组件处于非激活状态
	RadialForce->bAutoActivate = false;         //禁止自动激活
	RadialForce->ForceStrength = 3000000.f;  //设置爆炸的力量
	RadialForce->Radius = 500.f;                      //设置爆炸半径

	IsBoom = false;
	CanBoom = true;
}

// Called when the game starts or when spawned
void ABoomActor::BeginPlay()
{
	Super::BeginPlay();
	QueryChar->OnComponentBeginOverlap.AddDynamic(this, &ABoomActor::BeginOverlap);
}

// Called every frame
void ABoomActor::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
}

void ABoomActor::Boom()
{
	RadialForce->SetActive(true);
	QueryChar->SetCollisionEnabled(ECollisionEnabled::NoCollision);  //停止检测，已经爆炸过
	if (BoomEmitter)
	{
		//UGameplayStatics::SpawnEmitterAttached(BoomEmitter, WhatToBoom, NAME_None, WhatToBoom->GetComponentLocation());
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BoomEmitter, GetActorTransform());
	}
	WhatToBoom->SetVisibility(false);
	WhatToBoom->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	if (GetWorld())
	{
		GetWorldTimerManager().SetTimer(SpawnParticle, this, &ABoomActor::DestroyActor, 2.f, false);
		GetWorldTimerManager().SetTimer(ForceTime, this, &ABoomActor::StopForce, 0.5f, false);    //爆炸效果只持续0.5秒
	}

	IsBoom = true;
	CanBoom = false;
}

void ABoomActor::DestroyActor()
{
	Super::Destroy();
	GetWorldTimerManager().ClearTimer(SpawnParticle);
	GetWorldTimerManager().ClearTimer(ForceTime);
}

void ABoomActor::StopForce()
{
	RadialForce->SetActive(false);
}

void ABoomActor::StartSimulatePhysic()
{
	if (WhatToBoom != NULL)
		WhatToBoom->SetSimulatePhysics(true);
}

void ABoomActor::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (InitiativeToBoom)     //如果是主动爆炸（检测到人就爆炸）
	{
		if (Cast<AMyFirstGameCharacter>(OtherActor))
		{
			Boom(); //如果检测到玩家就爆炸
		}
	}
}