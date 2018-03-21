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
	QueryChar->SetCollisionObjectType(COLLISION_BOOMQUERY);    //���ü�������ײ����
	QueryChar->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	QueryChar->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	QueryChar->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECollisionResponse::ECR_Ignore);    //�����ӵ���ײ���
	QueryChar->SetCollisionEnabled(ECollisionEnabled::QueryOnly);      //ֻ���ڼ��

	WhatToBoom->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	WhatToBoom->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECollisionResponse::ECR_Block);   //��Ӧ�ӵ���ײ���
	RootComponent = WhatToBoom;

	QueryChar->SetupAttachment(WhatToBoom);
	WhatToBoom->SetRelativeLocation(FVector(0.f, 0.f, 0.f));  
	RadialForce->SetupAttachment(RootComponent);
	RadialForce->SetRelativeLocation(FVector::ZeroVector);
	RadialForce->SetActive(false);                   //Ĭ�����ñ�ը������ڷǼ���״̬
	RadialForce->bAutoActivate = false;         //��ֹ�Զ�����
	RadialForce->ForceStrength = 3000000.f;  //���ñ�ը������
	RadialForce->Radius = 500.f;                      //���ñ�ը�뾶

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
	QueryChar->SetCollisionEnabled(ECollisionEnabled::NoCollision);  //ֹͣ��⣬�Ѿ���ը��
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
		GetWorldTimerManager().SetTimer(ForceTime, this, &ABoomActor::StopForce, 0.5f, false);    //��ըЧ��ֻ����0.5��
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
	if (InitiativeToBoom)     //�����������ը����⵽�˾ͱ�ը��
	{
		if (Cast<AMyFirstGameCharacter>(OtherActor))
		{
			Boom(); //�����⵽��Ҿͱ�ը
		}
	}
}