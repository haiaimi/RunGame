// Fill out your copyright notice in the Description page of Project Settings.

#include "RunPlatform_Shoot.h"
#include "BoomActor.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"


ARunPlatform_Shoot::ARunPlatform_Shoot(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	IsShootToSlope = true;
	SlopeAngle = 30.f; 
}

void ARunPlatform_Shoot::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	/*爆炸触发器就放在两个平台的中间*/
	if (Trigger)
	{
		FVector SpawnDirX = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::X);
		FVector SpawnDirY = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Y);
		FVector SpawnDirZ = -FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Z);
		FVector SpawnLocation = GetActorLocation() + SpawnDirX * GetPlatformLength() + SpawnDirY * GetPlatformWidth() / 2 + SpawnDirZ * GetPlatformWidth() / 2;

		AimTrigger = GetWorld()->SpawnActor<ABoomActor>(Trigger, FTransform(FRotator::ZeroRotator, SpawnLocation));

		//下面就生成具有物理模拟的爆炸物
		InitiativeBoom = GetWorld()->SpawnActorDeferred<ABoomActor>(Trigger, FTransform(FRotator::ZeroRotator, GetActorLocation() + SpawnDirY * GetPlatformWidth() / 2));
		if (InitiativeBoom)
		{
			InitiativeBoom->InitiativeToBoom = true;
			UGameplayStatics::FinishSpawningActor(InitiativeBoom, FTransform(FRotator::ZeroRotator, GetActorLocation() + SpawnDirX * 20 + SpawnDirY * GetPlatformWidth() / 2 + SpawnDirZ * -20));
		}
	}
}

void ARunPlatform_Shoot::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	//如果爆炸就开始倾斜
	if(AimTrigger)
		if (AimTrigger->IsBoom)
		{
			IsSlope = true;
			InitiativeBoom->StartSimulatePhysic();
		}
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
}