// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "MyPlayerCameraManager.h"
#include "EngineUtils.h"
#include "RunPlatform.h"
#include "RunPlatform_Shoot.h"
#include "ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "MyFirstGame.h"

const float ShootPlatformAngle = 30.f;

AMyPlayerController::AMyPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PlayerCameraManagerClass = AMyPlayerCameraManager::StaticClass();
	static ConstructorHelpers::FClassFinder<ARunPlatform> SpawnPlat(TEXT("/Game/Blueprint/RunPlatform_BP"));
	static ConstructorHelpers::FClassFinder<ARunPlatform> SpawnPlat_Shoot(TEXT("/Game/Blueprint/RunPlatform_Shoot_BP"));
	SpawnPlatform = SpawnPlat.Class;
	SpawnPlatform_Shoot = SpawnPlat_Shoot.Class;
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
		Temp->PlatDir = AbsoluteDir;
		PlatformArray[i] = Temp;
	}
}

void AMyPlayerController::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	int32 Random_Shoot = FMath::Rand() % 100;  //��������������������Ƿ����ɴ���ƽ̨
	ARunPlatform* AddPlatform;   //ָ�򼴽����ɵ�ƽ̨

	if (CurPlatform != TempPlatform && CurPlatform != NULL) //�������ƽ̨�����仯
	{
		if (Random_Shoot >= 0 && Random_Shoot <= 24 && !Cast <ARunPlatform_Shoot>(PlatformArray.Last()))   //30%�ļ������ɴ�����ƽ̨,����ǰһ��ƽ̨���������
			AddPlatform = GetWorld()->SpawnActor<ARunPlatform_Shoot>(SpawnPlatform_Shoot, GetSpawnTransf_Shoot(PlatformArray.Last()));
		else
			AddPlatform = GetWorld()->SpawnActor<ARunPlatform>(SpawnPlatform, GetRandomSpawnTransf(PlatformArray.Last()));

		AddPlatform->PlatDir = AbsoluteDir;
		PlatformArray.Push(AddPlatform);
		TempPlatform = CurPlatform;   //
	}
}

FTransform AMyPlayerController::GetSpawnTransf_Shoot(ARunPlatform* PrePlatform)
{
	FTransform TempTrans;
	if (PrePlatform)
	{
		AbsoluteDir = PrePlatform->PlatDir;    //��ƽֻ̨���������ǰ���ƽ̨��ǰ��
		FVector CurLocation = PrePlatform->GetActorLocation();
		FRotator CurRotation = PrePlatform->GetActorRotation();

		FMatrix RotatMatrix = FRotationMatrix(CurRotation);
		FVector ForwardDir = -RotatMatrix.GetUnitAxis(EAxis::X);
		FVector UpDir = RotatMatrix.GetUnitAxis(EAxis::Z);
		float SlopeDegree = FMath::DegreesToRadians(ShootPlatformAngle);

		FVector SpawnLocation = CurLocation + FMath::Cos(SlopeDegree)*PrePlatform->GetPlatformLength()*ForwardDir + FMath::Sin(SlopeDegree)*PrePlatform->GetPlatformLength()*UpDir;
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