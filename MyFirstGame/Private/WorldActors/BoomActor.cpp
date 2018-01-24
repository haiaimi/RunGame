// Fill out your copyright notice in the Description page of Project Settings.

#include "BoomActor.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"


// Sets default values
ABoomActor::ABoomActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RadialForce = CreateDefaultSubobject<URadialForceComponent>("RadialForce");
	WhatToBoom = CreateDefaultSubobject<UStaticMeshComponent>("BoomMesh");
	BoomEmitter = CreateDefaultSubobject<UParticleSystem>("BoomEmitter");

	RootComponent = WhatToBoom;

	RadialForce->SetupAttachment(RootComponent);
	RadialForce->SetRelativeLocation(FVector::ZeroVector);
	RadialForce->SetActive(false);                   //默认设置爆炸组件处于非激活状态
	RadialForce->bAutoActivate = false;         //禁止自动激活
	RadialForce->ForceStrength = 100000.f;  //设置爆炸的力量
	RadialForce->Radius = 200.f;                      //设置爆炸半径

	CanBoom = true;
}

// Called when the game starts or when spawned
void ABoomActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABoomActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABoomActor::Boom()
{
	RadialForce->SetActive(true);

	if (BoomEmitter)
	{
		//UGameplayStatics::SpawnEmitterAttached(BoomEmitter, WhatToBoom, NAME_None, WhatToBoom->GetComponentLocation());
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BoomEmitter, GetActorTransform());
	}
	WhatToBoom->SetVisibility(false);
	
	if (GetWorld())
	{
		GetWorldTimerManager().SetTimer(SpawnParticle, this, &ABoomActor::DestroyActor, 2.f, false);
	}

	CanBoom = false;
}

void ABoomActor::DestroyActor()
{
	Destroy();
	GetWorldTimerManager().ClearTimer(SpawnParticle);
}
