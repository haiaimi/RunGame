// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "MyPlayerCameraManager.h"
#include "EngineUtils.h"
#include "RunPlatform.h"
#include "RunPlatform_Shoot.h"
#include "RunPlatform_Beam.h"
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
	static ConstructorHelpers::FClassFinder<ARunPlatform> FSpawnPlat_Beam(TEXT("/Game/Blueprint/RunPlatform_Beam_BP"));
	static ConstructorHelpers::FClassFinder<ABonus> FBonus_Score(TEXT("/Game/Blueprint/Bonus/Bonus_Score_BP"));

	SpawnPlatform = FSpawnPlat.Class;
	SpawnPlatform_Shoot = FSpawnPlat_Shoot.Class;
	SpawnPlatform_Beam = FSpawnPlat_Beam.Class;
	Bonus_Score = FBonus_Score.Class;

	CurrentWeaponType = EWeaponType::Weapon_Instant;
	InConnectedToPlat = false;
	CurConnectedPlat = NULL;
}


void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void AMyPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	PlatformArray.SetNum(10);       //ƽ̨���������Ϊ10

	//��ȡ��Ϸ������Ԥ���һ��ƽ̨����Ĭ�ϵ�һ����
	for (TActorIterator<ARunPlatform> It(GetWorld()); It; ++It)
	{
		if ((*It)->Tags.Num())
		{
			if ((*It)->Tags[0] == TEXT("StartPlatform"))   //�����ָ���Ŀ�ʼƽ̨�Ϳ�ʼ
			{
				CurPlatform = *It;
				TempPlatform = CurPlatform;
				PlatformArray[0] = *It;  //����ĵ�һ������Ĭ��ƽ̨
				GEngine->AddOnScreenDebugMessage(1, 5, FColor::Black, TEXT("�����ɹ�"));
				break;
			}
		}
	}

	//������������10��ƽ̨
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

	if (SpawnPlatform_Beam != NULL)
		GEngine->AddOnScreenDebugMessage(1, 5, FColor::Black, TEXT("�ҵ�Beam Platform"));
}

void AMyPlayerController::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	ARunPlatform* AddPlatform;   //ָ�򼴽����ɵ�ƽ̨
	if (CurPlatform != TempPlatform && CurPlatform != NULL && !PlatformArray.Last()->MoveToNew)   //�������ƽ̨�����仯���������һ��ƽ̨�����ƶ�ʱ
	{
		int32 Random_Shoot = FMath::Rand() % 100;  //��������������������Ƿ����ɴ���ƽ̨
		int32 Random_Bonus_Score = FMath::Rand() % 100;
		int32 Random_Beam = FMath::Rand() % 100;    //����������������Ƿ�������Ҫ����ǹ������ƽ̨

		if (Random_Beam >= 0 && Random_Beam < 20 && !Cast<ARunPlatform_Shoot>(PlatformArray.Last()) && !Cast<ARunPlatform_Beam>(PlatformArray.Last()))  //5%�ļ�����������ƽ̨��������һ��ƽ̨������������ͺ���������
		{
			AddPlatform = GetWorld()->SpawnActor<ARunPlatform_Beam>(SpawnPlatform_Beam, GetSpawnTransf_Beam(PlatformArray.Last()));
			PlatformArray.Last()->NoPlayerToSlope = true;   //��ƽ̨����һ��ƽ̨��������ת
			PlatformArray.Last()->SlopeAngle = 0.f;
		}
		else
		{
			if (Random_Shoot >= 0 && Random_Shoot < 25 && !Cast <ARunPlatform_Shoot>(PlatformArray.Last()) && !Cast<ARunPlatform_Beam>(PlatformArray.Last()))   //25%�ļ������ɴ�����ƽ̨,����ǰһ��ƽ̨��������ͺ�������
				AddPlatform = GetWorld()->SpawnActor<ARunPlatform_Shoot>(SpawnPlatform_Shoot, GetSpawnTransf_Shoot(PlatformArray.Last()));
			else
				AddPlatform = GetWorld()->SpawnActor<ARunPlatform>(SpawnPlatform, GetRandomSpawnTransf(PlatformArray.Last()));
		}

		if (AddPlatform)
		{
			AddPlatform->PlatDir = AbsoluteDir;
			PlatformArray.Last()->NextPlatform = AddPlatform;  //ָ����һ��ƽ̨
			PlatformArray.Push(AddPlatform);
			PlatformArray.Remove(TempPlatform);  //�Ƴ��Ѿ��߹���ƽ̨
			TempPlatform = CurPlatform;   //

			if (Random_Bonus_Score >= 0 && Random_Bonus_Score < 30)
				SpawnBonus_Score(AddPlatform);   //30�ļ�������Bonus Score
		}
	}
}

void AMyPlayerController::Destroyed()
{
	Super::Destroyed();

	int32 ArrayNum = PlatformArray.Num();
	//ɾ��ƽ̨�����ͷſռ�
	//for (int32 i = 0; i < ArrayNum; i++)
		//PlatformArray.RemoveAt(ArrayNum);
}


FTransform AMyPlayerController::GetSpawnTransf_Shoot(ARunPlatform* PrePlatform)
{
	FTransform TempTrans;
	if (PrePlatform)
	{
		AbsoluteDir = PrePlatform->PlatDir;    //��ƽֻ̨���������ǰ���ƽ̨��ǰ��
		FVector CurLocation = PrePlatform->SpawnLocation;
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
		FVector CurLocation = PrePlatform->SpawnLocation;
		FRotator CurRotation = PrePlatform->GetActorRotation();

		FMatrix RotatMatrix = FRotationMatrix(CurRotation);
		FVector ForwardDir = -RotatMatrix.GetUnitAxis(EAxis::X);
		FVector LeftDir = RotatMatrix.GetUnitAxis(EAxis::Y);
		FVector RightDir = -RotatMatrix.GetUnitAxis(EAxis::Y);
		FVector SpawnLocation;
		FRotator SpawnRotation;
		uint8 PreDir = PrePlatform->PlatDir;

		uint8 Dir = 0;    //������Է���
		
		if(PreDir == EPlatformDirection::Absolute_Forward)
			Dir = FMath::Rand() % 3;  //�����������

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
			
		if (Cast<ARunPlatform_Beam>(PrePlatform) != NULL)     //���ǰһ��ƽ̨������ƽ̨
		{
			Dir = 1;  //ֻ������ǰһ�������ǰ��
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

		/**������ǵó���һ������ƽ̨�ľ������緽��*/
		if (PrePlatform->PlatDir == EPlatformDirection::Absolute_Forward)
			AbsoluteDir = Dir;

		else if (PrePlatform->PlatDir == EPlatformDirection::Absolute_Left)
			AbsoluteDir = (Dir == 1) ? EPlatformDirection::Absolute_Left : EPlatformDirection::Absolute_Forward;

		else if (PrePlatform->PlatDir == EPlatformDirection::Absolute_Right)
			AbsoluteDir = (Dir == 0) ? EPlatformDirection::Absolute_Forward : EPlatformDirection::Absolute_Right;
	}
	return TempTrans;
}

FTransform AMyPlayerController::GetSpawnTransf_Beam(ARunPlatform* PrePlatform)
{
	FTransform TempTrans;
	if (PrePlatform)
	{
		AbsoluteDir = PrePlatform->PlatDir;    //��ƽֻ̨���������ǰ���ƽ̨��ǰ��
		FVector CurLocation = PrePlatform->SpawnLocation;
		FRotator CurRotation = PrePlatform->GetActorRotation();

		FMatrix RotatMatrix = FRotationMatrix(CurRotation);
		FVector ForwardDir = -RotatMatrix.GetUnitAxis(EAxis::X);

		FVector SpawnLocation = CurLocation + ForwardDir * PrePlatform->GetPlatformLength() * 3;
		TempTrans = FTransform(CurRotation, SpawnLocation);
	}
	return TempTrans;
}

void AMyPlayerController::SpawnBonus_Score(ARunPlatform* const CurPlatform)
{
	if (CurPlatform && Bonus_Score)
	{
		FMatrix Orient = FRotationMatrix(CurPlatform->GetActorRotation());    //ƽ̨����
		FVector SpawnDirX = Orient.GetUnitAxis(EAxis::X);
		FVector SpawnDirY = Orient.GetUnitAxis(EAxis::Y);
		FVector SpawnDirZ = Orient.GetUnitAxis(EAxis::Z);

		FVector FirstSpawnLoc;
		int32 ScorePosOnPlat = FMath::Rand() % 3;     //0Ϊ��1Ϊ�У�2Ϊ��
		//���������������λ�õ�Score
		if (ScorePosOnPlat == 0)
			FirstSpawnLoc = CurPlatform->GetActorLocation() + SpawnDirY * (CurPlatform->GetPlatformWidth() - 20) + 30 * SpawnDirX + SpawnDirZ * 70;
		else if (ScorePosOnPlat == 1)
			FirstSpawnLoc = CurPlatform->GetActorLocation() + SpawnDirY * CurPlatform->GetPlatformWidth() / 2 + 30 * SpawnDirX + SpawnDirZ * 70;
		else if (ScorePosOnPlat == 2)
			FirstSpawnLoc = CurPlatform->GetActorLocation() + SpawnDirY * 20 + 30 * SpawnDirX + SpawnDirZ * 70;

		int32 CurPlatformIndex, BonusNum = 0;

		if (PlatformArray.Find(CurPlatform, CurPlatformIndex))
		{
			if (CurPlatform->PlatDir == PlatformArray[CurPlatformIndex - 1]->PlatDir)
				BonusNum = CurPlatform->GetPlatformLength() / 100.f;
			else
				BonusNum = CurPlatform->GetPlatformLength() / 100.f - 1;
		}

		for (int32 i = 0; i < BonusNum; i++)
		{
			ABonus* SpawnBonus = GetWorld()->SpawnActor<ABonus>(Bonus_Score, FTransform(CurPlatform->GetActorRotation(), FirstSpawnLoc + 100 * i * SpawnDirX));
			
			SpawnBonus->AttachToActor(CurPlatform, FAttachmentTransformRules::KeepWorldTransform);
			CurPlatform->OnDestory.AddUObject(SpawnBonus, &ABonus::DestroyActor);
			CurPlatform->OnFall.AddUObject(SpawnBonus, &ABonus::StartFall);
		}
	}
}

void AMyPlayerController::ChangeWeaponType(EWeaponType::Type WeaponType)
{
	CurrentWeaponType = WeaponType;
	if (CurrentWeaponType != EWeaponType::Weapon_Beam && InConnectedToPlat && CurConnectedPlat != NULL)
	{
		CurConnectedPlat->DeActiveBeam();
		CurConnectedPlat = NULL;
	}
}