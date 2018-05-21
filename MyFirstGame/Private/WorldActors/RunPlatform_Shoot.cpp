﻿// Fill out your copyright notice in the Description page of Project Settings.

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

	/*爆炸触发器就放在两个平台的中间*/
	if (Trigger)
	{
		const FVector SpawnDirX = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::X);
		const FVector SpawnDirY = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Y);
		const FVector SpawnDirZ = -FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Z);
		FVector SpawnLocation = GetActorLocation() + SpawnDirX * GetPlatformLength() + SpawnDirY * GetPlatformWidth() / 2 + SpawnDirZ * GetPlatformWidth() / 2;

		AimTrigger = GetWorld()->SpawnActor<ABoomActor>(Trigger, FTransform(FRotator::ZeroRotator, SpawnLocation));
		AimTrigger->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		//下面就生成具有物理模拟的爆炸物
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
	//如果爆炸就开始倾斜
	if (AimTrigger != nullptr)
		if (AimTrigger->IsBoom && !AimTrigger->CanBoom && InitiativeBoom != NULL)   //CanBoom是用来确定BoomActor是否已经炸过
		{
			IsSlope = true;
			InitiativeBoom->StartSimulatePhysic();
			InitiativeBoom = nullptr;
			AimTrigger = nullptr;
		}

	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
}

void ARunPlatform_Shoot::MoveToAllTick(float DeltaTime)
{
	Super::MoveToAllTick(DeltaTime);

	//if (MoveToAll && !MoveToNew)
	//{
	//	if (GetActorRotation().Pitch < -1.f)   //如果该平台已经旋转过
	//	{
	//		const float NewPitch = FMath::FInterpTo(GetActorRotation().Pitch, 0, DeltaTime, 10.f);
	//		SetActorRotation(FRotator(NewPitch, GetActorRotation().Yaw, GetActorRotation().Roll));
	//	}
	//}
}

void ARunPlatform_Shoot::StartDestroy()
{
	Super::StartDestroy();

	//用于清除两个炸弹
	if (AimTrigger != nullptr)
		AimTrigger->DestroyActor();

	if (InitiativeBoom != nullptr)
		InitiativeBoom->DestroyActor();
}

void ARunPlatform_Shoot::MoveToAllFun(const FVector DeltaDistance)
{
	Super::MoveToAllFun(DeltaDistance);

	DstRotation = FRotator(0.f, GetActorRotation().Yaw, GetActorRotation().Roll);
	IsSlope = true;

	if (InitiativeBoom)
	{
		const FVector SpawnDirX = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::X);
		const FVector SpawnDirY = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Y);
		const FVector SpawnDirZ = -FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Z);
		InitiativeBoom->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		InitiativeBoom->SetActorLocation(GetActorLocation() + SpawnDirY * GetPlatformWidth() / 2 + SpawnDirZ * 400.f);
		InitiativeBoom->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}

}

void ARunPlatform_Shoot::StopToAllFun(const FVector DeltaDistance)
{
	Super::StopToAllFun(DeltaDistance);

	if (InitiativeBoom)
	{
		const FVector SpawnDirX = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::X);
		const FVector SpawnDirY = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Y);
		const FVector SpawnDirZ = -FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Z);
		InitiativeBoom->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		InitiativeBoom->SetActorLocation(GetActorLocation() + SpawnDirY * GetPlatformWidth() / 2 - SpawnDirZ * 20.f + SpawnDirX * 20);
		InitiativeBoom->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	}

	NoPlayerToSlope = true;
	IsSlope = false;
	DstRotation = FRotator(-SlopeAngle, GetActorRotation().Yaw, GetActorRotation().Roll);

	if (AimTrigger == nullptr)
	{
		IsSlope = true;
	}
}