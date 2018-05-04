// Fill out your copyright notice in the Description page of Project Settings.

#include "JumpPlatform.h"
#include "Components/StaticMeshComponent.h"
#include "MyFirstGameCharacter.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

AJumpPlatform::AJumpPlatform(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	NoPlayerToSlope = false;     //需要玩家触发旋转
	XScale = 1.f;
	YScale = 1.f;
	SlopeAngle = 30.f;
}

void AJumpPlatform::BeginPlay()
{
	Super::BeginPlay();
}

void AJumpPlatform::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction & ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
}

void AJumpPlatform::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SetActorRotation(FRotator(0.f, 0.f, -30.f));
	DstRotation = FRotator(0.f, 0.f, 0.f);
}

void AJumpPlatform::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (Cast<AMyFirstGameCharacter>(OtherActor))
	{
		IsSlope = true;
		GetWorldTimerManager().SetTimer(ToFall, this, &ARunPlatform::StartDestroy, 2.f);    //两秒后坠落
	}
}

void AJumpPlatform::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<AMyFirstGameCharacter>(OtherActor))
	{
		IsSlope = false;
	}
}
