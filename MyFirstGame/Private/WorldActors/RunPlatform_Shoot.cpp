// Fill out your copyright notice in the Description page of Project Settings.

#include "RunPlatform_Shoot.h"
#include "BoomActor.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"


ARunPlatform_Shoot::ARunPlatform_Shoot(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	NoPlayerToSlope = true;
	SlopeAngle = 30.f; 
}

void ARunPlatform_Shoot::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	/*��ը�������ͷ�������ƽ̨���м�*/
	if (Trigger)
	{
		const FVector SpawnDirX = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::X);
		const FVector SpawnDirY = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Y);
		const FVector SpawnDirZ = -FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Z);
		FVector SpawnLocation = GetActorLocation() + SpawnDirX * GetPlatformLength() + SpawnDirY * GetPlatformWidth() / 2 + SpawnDirZ * GetPlatformWidth() / 2;

		AimTrigger = GetWorld()->SpawnActor<ABoomActor>(Trigger, FTransform(FRotator::ZeroRotator, SpawnLocation));
		AimTrigger->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		//��������ɾ�������ģ��ı�ը��
		InitiativeBoom = GetWorld()->SpawnActorDeferred<ABoomActor>(Trigger, FTransform(FRotator::ZeroRotator, GetActorLocation() + SpawnDirY * GetPlatformWidth() / 2));
		if (InitiativeBoom)
		{
			InitiativeBoom->InitiativeToBoom = true;
			UGameplayStatics::FinishSpawningActor(InitiativeBoom, FTransform(FRotator::ZeroRotator, GetActorLocation() + SpawnDirX * 20 + SpawnDirY * GetPlatformWidth() / 2 + SpawnDirZ * -20));
		}
		InitiativeBoom->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}
}

void ARunPlatform_Shoot::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	//�����ը�Ϳ�ʼ��б
	if (AimTrigger != NULL)
		if (AimTrigger->IsBoom && !AimTrigger->CanBoom && InitiativeBoom != NULL)   //CanBoom������ȷ��BoomActor�Ƿ��Ѿ�ը��
		{
			IsSlope = true;
			InitiativeBoom->StartSimulatePhysic();
		}

	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
}

void ARunPlatform_Shoot::MoveToAllTick(float DeltaTime)
{
	Super::MoveToAllTick(DeltaTime);

	if (MoveToAll && !MoveToNew)
	{
		if (GetActorRotation().Pitch < -1.f)   //�����ƽ̨�Ѿ���ת��
		{
			const float NewPitch = FMath::FInterpTo(GetActorRotation().Pitch, 0, DeltaTime, 10.f);
			SetActorRotation(FRotator(NewPitch, GetActorRotation().Yaw, GetActorRotation().Roll));
		}
	}
}

void ARunPlatform_Shoot::MoveToAllFun(const FVector DeltaDistance)
{
	IsSlope = false;
	DeltaLoc = DeltaDistance;
	MoveToAll = true;

	if (InitiativeBoom)
	{
		const FVector SpawnDirX = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::X);
		const FVector SpawnDirY = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Y);
		const FVector SpawnDirZ = -FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Z);
		InitiativeBoom->SetActorRelativeLocation(SpawnDirY * GetPlatformWidth() / 2 + SpawnDirZ * (-100.f));
	}
}