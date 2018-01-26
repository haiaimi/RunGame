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

	/*��ը�������ͷ�������ƽ̨���м�*/
	if (Trigger)
	{
		FVector SpawnDirX = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::X);
		FVector SpawnDirY = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Y);
		FVector SpawnDirZ = -FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Z);
		FVector SpawnLocation = GetActorLocation() + SpawnDirX * GetPlatformLength() + SpawnDirY * GetPlatformWidth() / 2 + SpawnDirZ * GetPlatformWidth() / 2;

		AimTrigger = GetWorld()->SpawnActor<ABoomActor>(Trigger, FTransform(FRotator::ZeroRotator, SpawnLocation));

		//��������ɾ�������ģ��ı�ը��
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
	//�����ը�Ϳ�ʼ��б
	if(AimTrigger)
		if (AimTrigger->IsBoom)
		{
			IsSlope = true;
			InitiativeBoom->StartSimulatePhysic();
		}
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
}