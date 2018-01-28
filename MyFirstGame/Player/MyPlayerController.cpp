// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "MyPlayerCameraManager.h"
#include "EngineUtils.h"
#include "RunPlatform.h"
#include "RunPlatform_Shoot.h"
#include "ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "MyFirstGame.h"
#include "Bonus.h"

const float ShootPlatformAngle = 30.f;

AMyPlayerController::AMyPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PlayerCameraManagerClass = AMyPlayerCameraManager::StaticClass();
	static ConstructorHelpers::FClassFinder<ARunPlatform> FSpawnPlat(TEXT("/Game/Blueprint/RunPlatform_BP"));
	static ConstructorHelpers::FClassFinder<ARunPlatform> FSpawnPlat_Shoot(TEXT("/Game/Blueprint/RunPlatform_Shoot_BP"));
	static ConstructorHelpers::FClassFinder<ABonus> FBonus_Score(TEXT("/Game/Blueprint/Bonus/Bonus_Score_BP"));
	SpawnPlatform = FSpawnPlat.Class;
	SpawnPlatform_Shoot = FSpawnPlat_Shoot.Class;
	Bonus_Score = FBonus_Score.Class;
}


void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void AMyPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	PlatformArray.SetNum(10);       //平台数组的容量为10

	//获取游戏世界中预设的一个平台，（默认的一个）
	for (TActorIterator<ARunPlatform> It(GetWorld()); It; ++It)
	{
		if ((*It)->Tags.Num())
		{
			if ((*It)->Tags[0] == TEXT("StartPlatform"))   //如果是指定的开始平台就开始
			{
				CurPlatform = *It;
				TempPlatform = CurPlatform;
				PlatformArray[0] = *It;  //数组的第一个就是默认平台
				GEngine->AddOnScreenDebugMessage(1, 5, FColor::Black, TEXT("检索成功"));
				break;
			}
		}
	}

	//下面是逐步生成10个平台
	int32 ArrayNum = PlatformArray.Num();
	for (int32 i = 1; i < ArrayNum; i++)
	{
		ARunPlatform* Temp = GetWorld()->SpawnActor<ARunPlatform>(SpawnPlatform, GetRandomSpawnTransf(PlatformArray[i - 1]));
		if (Temp)
		{
			Temp->PlatDir = AbsoluteDir;
			PlatformArray[i] = Temp;
			PlatformArray[i - 1]->NextPlatform = PlatformArray[i];
		}
	}
}

void AMyPlayerController::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	int32 Random_Shoot = FMath::Rand() % 100;  //生成随机数，用来决定是否生成触发平台
	int32 Random_Bonus_Score = FMath::Rand() % 100;
	ARunPlatform* AddPlatform;   //指向即将生成的平台

	if (CurPlatform != TempPlatform && CurPlatform != NULL)   //玩家所在平台发生变化
	{
		if (Random_Shoot >= 0 && Random_Shoot <= 24 && !Cast <ARunPlatform_Shoot>(PlatformArray.Last()))   //30%的几率生成触发型平台,并且前一个平台不是射击型
			AddPlatform = GetWorld()->SpawnActor<ARunPlatform_Shoot>(SpawnPlatform_Shoot, GetSpawnTransf_Shoot(PlatformArray.Last()));
		else
			AddPlatform = GetWorld()->SpawnActor<ARunPlatform>(SpawnPlatform, GetRandomSpawnTransf(PlatformArray.Last()));

		AddPlatform->PlatDir = AbsoluteDir;
		PlatformArray.Last()->NextPlatform = AddPlatform;  //指定下一个平台
		PlatformArray.Push(AddPlatform);
		TempPlatform = CurPlatform;   //

		if (Random_Bonus_Score >= 0 && Random_Bonus_Score < 50)
			SpawnBonus_Score(AddPlatform);   //50的几率生成Bonus Score
	}
}

FTransform AMyPlayerController::GetSpawnTransf_Shoot(ARunPlatform* PrePlatform)
{
	FTransform TempTrans;
	if (PrePlatform)
	{
		AbsoluteDir = PrePlatform->PlatDir;    //该平台只生成在相对前面的平台的前方
		FVector CurLocation = PrePlatform->GetActorLocation();
		FRotator CurRotation = PrePlatform->GetActorRotation();

		FMatrix RotatMatrix = FRotationMatrix(CurRotation);
		FVector ForwardDir = -RotatMatrix.GetUnitAxis(EAxis::X);
		FVector UpDir = RotatMatrix.GetUnitAxis(EAxis::Z);
		float SlopeRadians = FMath::DegreesToRadians(ShootPlatformAngle);

		FVector SpawnLocation = CurLocation + FMath::Cos(SlopeRadians)*PrePlatform->GetPlatformLength()*ForwardDir + FMath::Sin(SlopeRadians)*PrePlatform->GetPlatformLength()*UpDir;
		TempTrans = FTransform(CurRotation, SpawnLocation);
	}
	return TempTrans;
}

FTransform AMyPlayerController::GetRandomSpawnTransf(ARunPlatform* PrePlatform)
{
	FTransform TempTrans;
	if (PrePlatform)
	{
		FVector CurLocation = PrePlatform->GetActorLocation();
		FRotator CurRotation = PrePlatform->GetActorRotation();

		FMatrix RotatMatrix = FRotationMatrix(CurRotation);
		FVector ForwardDir = -RotatMatrix.GetUnitAxis(EAxis::X);
		FVector LeftDir = RotatMatrix.GetUnitAxis(EAxis::Y);
		FVector RightDir = -RotatMatrix.GetUnitAxis(EAxis::Y);
		FVector SpawnLocation;
		FRotator SpawnRotation;
		uint8 PreDir = PrePlatform->PlatDir;

		uint8 Dir = 0;
		
		if(PreDir==EPlatformDirection::Absolute_Forward)
			Dir = FMath::Rand() % 3;  //随机三个方向

		if (PreDir == EPlatformDirection::Absolute_Left)
		{
			Dir = FMath::RandBool();
			Dir = (Dir == false) ? 1 : 2;
		}

		if (PreDir == EPlatformDirection::Absolute_Right)
		{
			Dir = FMath::RandBool();
			Dir = (Dir == false) ? 0 : 1;
		}
			
		switch (Dir)
		{
			case 0:
				SpawnLocation = CurLocation + ForwardDir * PrePlatform->GetPlatformWidth() + LeftDir * PrePlatform->GetPlatformLength();
				SpawnRotation = CurRotation - FRotator(0.f, 90.f, 0.f);
				TempTrans = FTransform(SpawnRotation, SpawnLocation);
				break;

			case 1:
				SpawnLocation = CurLocation + ForwardDir * PrePlatform->GetPlatformLength();
				SpawnRotation = CurRotation;
				TempTrans = FTransform(SpawnRotation, SpawnLocation);
				break;

			case 2:
				SpawnLocation = CurLocation + RightDir * (PrePlatform->GetPlatformLength() - PrePlatform->GetPlatformWidth());
				SpawnRotation = CurRotation + FRotator(0.f, 90.f, 0.f);
				TempTrans = FTransform(SpawnRotation, SpawnLocation);
				break;
		}
		/**下面就是得出下一个生成平台的绝对世界方向*/
		if (PrePlatform->PlatDir == EPlatformDirection::Absolute_Forward)
			AbsoluteDir = Dir;

		else if (PrePlatform->PlatDir == EPlatformDirection::Absolute_Left)
			AbsoluteDir = (Dir == 1) ? EPlatformDirection::Absolute_Left : EPlatformDirection::Absolute_Forward;

		else if (PrePlatform->PlatDir == EPlatformDirection::Absolute_Right)
			AbsoluteDir = (Dir == 0) ? EPlatformDirection::Absolute_Forward : EPlatformDirection::Absolute_Right;
	}
	return TempTrans;
}

void AMyPlayerController::SpawnBonus_Score(ARunPlatform* const CurPlatform)
{
	if (CurPlatform && Bonus_Score)
	{
		FMatrix Orient = FRotationMatrix(CurPlatform->GetActorRotation());    //平台方向
		FVector SpawnDirX = Orient.GetUnitAxis(EAxis::X);
		FVector SpawnDirY = Orient.GetUnitAxis(EAxis::Y);
		FVector SpawnDirZ = Orient.GetUnitAxis(EAxis::Z);

		FVector FirstSpawnLoc = CurPlatform->GetActorLocation() + SpawnDirY * CurPlatform->GetPlatformWidth() / 2 + 50 * SpawnDirX + SpawnDirZ * 70;
		int32 BonusNum = CurPlatform->GetPlatformLength() / 100.f;
		for (int32 i = 0; i < BonusNum; i++)
		{
			ABonus* SpawnBonus = GetWorld()->SpawnActor<ABonus>(Bonus_Score, FTransform(CurPlatform->GetActorRotation(), FirstSpawnLoc + 100 * i * SpawnDirX));
			
			SpawnBonus->AttachToActor(CurPlatform, FAttachmentTransformRules::KeepWorldTransform);
			CurPlatform->OnDestory.AddUObject(SpawnBonus, &ABonus::DestroyActor);
			CurPlatform->OnFall.AddUObject(SpawnBonus, &ABonus::StartFall);
		}
	}
}