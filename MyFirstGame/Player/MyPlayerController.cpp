// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "MyPlayerCameraManager.h"
#include "EngineUtils.h"
#include "RunPlatform.h"
#include "RunPlatform_Shoot.h"
#include "RunPlatform_Beam.h"
#include "RunPlatform_Physic.h"
#include "ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "MyFirstGame.h"
#include "Bonus.h"
#include "Kismet/GameplayStatics.h"
#include "Bullet.h"
#include "FlyObstacle.h"
#include "GameFramework/GameModeBase.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "RunGameState.h"
#include "GameFramework/CharacterMovementComponent.h"

const float ShootPlatformAngle = 30.f;

AMyPlayerController::AMyPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PlayerCameraManagerClass = AMyPlayerCameraManager::StaticClass();
	static ConstructorHelpers::FClassFinder<ARunPlatform> FSpawnPlat(TEXT("/Game/Blueprint/RunPlatform_BP"));
	static ConstructorHelpers::FClassFinder<ARunPlatform> FSpawnPlat_Shoot(TEXT("/Game/Blueprint/RunPlatform_Shoot_BP"));
	static ConstructorHelpers::FClassFinder<ARunPlatform> FSpawnPlat_Beam(TEXT("/Game/Blueprint/RunPlatform_Beam_BP"));
	static ConstructorHelpers::FClassFinder<ARunPlatform> FSpawnPlat_Physic(TEXT("/Game/Blueprint/RunPlatform_Physic_BP"));
	static ConstructorHelpers::FClassFinder<ABonus> FBonus_Score(TEXT("/Game/Blueprint/Bonus/Bonus_Score_BP"));
	static ConstructorHelpers::FClassFinder<ABonus> FBonus_NoObstacle(TEXT("/Game/Blueprint/Bonus/Bonus_NoObstacle_BP"));
	static ConstructorHelpers::FClassFinder<AFlyObstacle> FFlyObstacle(TEXT("/Game/Blueprint/FlyObstacle_BP"));

	SpawnPlatform = FSpawnPlat.Class;
	SpawnPlatform_Shoot = FSpawnPlat_Shoot.Class;
	SpawnPlatform_Beam = FSpawnPlat_Beam.Class;
	SpawnPlatform_Physic = FSpawnPlat_Physic.Class;
	Bonus_Score = FBonus_Score.Class;
	Bonus_NoObstacle = FBonus_NoObstacle.Class;
	SpawnFlyObstacle = FFlyObstacle.Class;

	CurrentWeaponType = EWeaponType::Weapon_Instant;
	InConnectedToPlat = false;
	CurConnectedPlat = nullptr;
	FlyObstacleSpawnInterval = -1;
	MaxFlyObstacles = 1;     //��Ϸ��ʼʱ�����ϰ���Ϊ0
	CurSpawnedShootPlats = 0;
	IsInPause = 0;
	SpawnNoObsBonusParam = 0;
}


void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction("TogglePause", IE_Pressed, this, &AMyPlayerController::TogglePauseStat);
	InputComponent->BindAction("TestToAll", IE_Pressed, this, &AMyPlayerController::StartToAllTest);
	InputComponent->BindAction("StopToAll", IE_Pressed, this, &AMyPlayerController::StopToAll);
}

void AMyPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	PlatformArray.SetNum(10);       //ƽ̨���������Ϊ10
	FlyObstacleArray.SetNum(0);

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
				//GEngine->AddOnScreenDebugMessage(1, 5, FColor::Black, TEXT("�����ɹ�"));
				break;
			}
		}
	}

	//������������10��ƽ̨
	int32 ArrayNum = PlatformArray.Num();
	for (int32 i = 1; i < ArrayNum; i++)
	{
		ARunPlatform* Temp = GetWorld()->SpawnActor<ARunPlatform>(SpawnPlatform, GetRandomSpawnTransf(PlatformArray[i - 1]));
	
		if (Temp != nullptr)
		{
			Temp->PlatDir = AbsoluteDir;
			PlatformArray[i] = Temp;
			PlatformArray[i - 1]->NextPlatform = PlatformArray[i];
		}
	}
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AMyPlayerController::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (PlatformArray.Num() > 0)
	{
		ARunPlatform* LastPlatformRef = PlatformArray.Last();
		if (LastPlatformRef != nullptr)
		{
			if (TempPlatform != CurPlatform && !PlatformArray.Last()->MoveToNew && CurPlatform != nullptr)   //�������ƽ̨�����仯���������һ��ƽ̨�����ƶ�ʱ
			{
				int32 CurPlatIndex = PlatformArray.Find(CurPlatform);     //��ǰ����ƽ̨�������е�λ��
				
				ARunGameState* RGS = Cast<ARunGameState>(GetWorld()->GetGameState());

				if (RGS)
				{
					RGS->AddPlayerDistance((CurPlatform->GetActorLocation() - TempPlatform->GetActorLocation()).Size2D());

					if (CurPlatform->IsA(SpawnPlatform_Shoot))
					{
						RGS->AddPlayerHeight(CurPlatform->GetActorLocation().Z);
					}
				}
				//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::Printf(TEXT("Player Distance: %f, Template Lenght: %f, DeltaDistance: %f"), PlayerMoveDistance, TempPlatform->GetPlatformLength(), (CurPlatform->GetActorLocation() - TempPlatform->GetActorLocation()).Size2D()));
				if (!IsInStopToAllAnim)
					RandomSpawnPlatform(CurPlatIndex);
			}

			if (!IsInPause)
			{
				AMyFirstGameCharacter* MC = Cast<AMyFirstGameCharacter>(GetPawn());
				if (MC)
				{
					float FallDistance = LastPlatformRef->GetActorLocation().Z - MC->GetActorLocation().Z;      //���������룬����һ����������Ϊ��Ϸ����
					if (FallDistance > 2000.f)
					{
						bIsGameEnd = true;
						TogglePauseStat();
					}
				}
			}
		}
	}
	
	int32 PlatNum = PlatformArray.Num();
	for (int32 i = 0; i < PlatNum; i++)
	{
		if (PlatformArray[i] != nullptr)
			if (PlatformArray[i]->IsInDestroyed)
			{
				PlatformArray[i] = nullptr;
				UE_LOG(LogRunGame, Log, TEXT("CurPlatNum: %d"),PlatformArray.Num())
			}
	}
}

void AMyPlayerController::Destroyed()
{
	Super::Destroyed();
}


void AMyPlayerController::RandomSpawnPlatform(int32 SpawnNum)
{
	ARunPlatform* AddPlatform = nullptr;   //ָ�򼴽����ɵ�ƽ̨

	for (int32 i = 0; i < SpawnNum; ++i)
	{
		int32 Random_Shoot = FMath::Rand() % 100;  //��������������������Ƿ����ɴ���ƽ̨
		int32 Random_Beam = FMath::Rand() % 100;    //����������������Ƿ�������Ҫ����ǹ������ƽ̨
		int32 Random_Physic = FMath::Rand() % 100;   //��������������������Ƿ���������ƽ̨
		int32 Random_Bonus_Score = FMath::Rand() % 100;

		//�������������ɲ��֣����ȼ���Beam > Physic > Shoot > Normal(����ƽ̨)
		if (Random_Beam >= 0 && Random_Beam < 10 && PlatformArray.Last()->IsA(SpawnPlatform)/*!Cast<ARunPlatform_Shoot>(PlatformArray.Last()) && !Cast<ARunPlatform_Beam>(PlatformArray.Last())*/)  //5%�ļ�����������ƽ̨��������һ��ƽ̨������������ͺ���������
		{
			
			AddPlatform = GetWorld()->SpawnActor<ARunPlatform_Beam>(SpawnPlatform_Beam, GetSpawnTransf_Beam(PlatformArray.Last()));
			PlatformArray.Last()->NoPlayerToSlope = true;   //��ƽ̨����һ��ƽ̨��������ת
			PlatformArray.Last()->SlopeAngle = 0.f;
		}
		else
		{
			if (Random_Physic >= 0 && Random_Physic < 10 && PlatformArray.Last()->IsA(SpawnPlatform))     //10%�ļ�����������ƽ̨
				AddPlatform = GetWorld()->SpawnActor<ARunPlatform_Physic>(SpawnPlatform_Physic, GetSpawnTransf_Physic(PlatformArray.Last()));
			else
			{
				if (Random_Shoot >= 0 && Random_Shoot < 10 && PlatformArray.Last()->IsA(SpawnPlatform)/*!Cast <ARunPlatform_Shoot>(PlatformArray.Last()) && !Cast<ARunPlatform_Beam>(PlatformArray.Last())*/)   //25%�ļ������ɴ�����ƽ̨,����ǰһ��ƽ̨��������ͺ�������
					AddPlatform = GetWorld()->SpawnActor<ARunPlatform_Shoot>(SpawnPlatform_Shoot, GetSpawnTransf_Shoot(PlatformArray.Last()));
				else
					AddPlatform = GetWorld()->SpawnActor<ARunPlatform>(SpawnPlatform, GetRandomSpawnTransf(PlatformArray.Last()));
			}
		}

		if (AddPlatform != nullptr)
		{
			AddPlatform->PlatDir = AbsoluteDir;
			PlatformArray.Last()->NextPlatform = AddPlatform;  //ָ����һ��ƽ̨
			PlatformArray.Push(AddPlatform);

			if (PlatformArray[0] != nullptr)
				if (!PlatformArray[0]->IsInDestroyed)
					PlatformArray[0]->StartDestroy();  //��ʼɾ����һ��ƽ̨
			
			PlatformArray[0] = nullptr;
			PlatformArray.RemoveAt(0);  //�Ƴ��Ѿ��߹���ƽ̨
			TempPlatform = CurPlatform;   //
			if (IsToAll)
				NewSpawnedPlatformToAll(AddPlatform);

			if (Random_Bonus_Score >= 0 && Random_Bonus_Score < 30)
				SpawnBonus_Score(AddPlatform);   //30�ļ�������Bonus Score

			if (AddPlatform->IsA(SpawnPlatform_Shoot))
				AddMaxSpawnObstacles();

			AddPlatform->DeltaLocToPrePlat = DeltaLocToPrePlat;

			if (!IsToAll)
			{
				//RandomSpawnFlyObstacle();    //ֻ���ڷ����ϰ�ģʽ�²�������ɷ����ϰ�
				SpawnNoObsBonusParam++;

				if (SpawnNoObsBonusParam >= PlatformArray.Num() * 3)
				{
					SpawnBonus_NoObstacle(AddPlatform);
					SpawnNoObsBonusParam = 0;
				}
			}

			int32 NoNullNum = 0;
			for (int32 i = 0; i < 10; ++i)
			{
				if (PlatformArray[i] != nullptr)
					NoNullNum++;
			}
			if (AddPlatform->IsA(SpawnPlatform_Beam))
				HasSpawnedBeamPlatNum++;

			HasSpawnedPlatNum++;
			UE_LOG(LogRunGame, Log, TEXT("PlatformArray num:%d, CurGameTime:%f, HasSpwanedPlatNum: %d, HasSpawnedBeamPlatNum: %d"), NoNullNum, GetWorld()->TimeSeconds, HasSpawnedPlatNum, HasSpawnedBeamPlatNum)
		}
		UE_LOG(LogRunGame, Log, TEXT("InSpwanPlatTick"))
	}
	UE_LOG(LogRunGame, Log, TEXT("OutSpwanPlatTick"))
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

		const FVector DelLoc = FMath::Cos(SlopeRadians)*PrePlatform->GetPlatformLength()*ForwardDir + FMath::Sin(SlopeRadians)*PrePlatform->GetPlatformLength()*UpDir;
		FVector SpawnLocation = CurLocation + DelLoc;
		TempTrans = FTransform(CurRotation, SpawnLocation);
		DeltaLocToPrePlat = DelLoc;
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
			
		int32 LengthScale = 1;    //�������ɵ�λ��
		if (PrePlatform->IsA(SpawnPlatform_Beam) || PrePlatform->IsA(SpawnPlatform_Physic))   //���ǰһ��ƽ̨������ƽ̨
		{
			Dir = 1;  //ֻ������ǰһ�������ǰ��
			if (PrePlatform->IsA(SpawnPlatform_Physic))
				LengthScale = FMath::Rand() % 3 + 4;
		}

		switch (Dir)
		{
			case 0:
				SpawnLocation = CurLocation + ForwardDir * PrePlatform->GetPlatformWidth() + LeftDir * PrePlatform->GetPlatformLength();
				SpawnRotation = CurRotation - FRotator(0.f, 90.f, 0.f);
				TempTrans = FTransform(SpawnRotation, SpawnLocation);
				break;

			case 1:
				SpawnLocation = CurLocation + ForwardDir * PrePlatform->GetPlatformLength() * LengthScale;
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
			AbsoluteDir = (EPlatformDirection::Type)Dir;

		else if (PrePlatform->PlatDir == EPlatformDirection::Absolute_Left)
			AbsoluteDir = (Dir == 1) ? EPlatformDirection::Absolute_Left : EPlatformDirection::Absolute_Forward;

		else if (PrePlatform->PlatDir == EPlatformDirection::Absolute_Right)
			AbsoluteDir = (Dir == 0) ? EPlatformDirection::Absolute_Forward : EPlatformDirection::Absolute_Right;

		if (PrePlatform->IsA(SpawnPlatform_Physic))      //ֻ��ǰһ��ƽ̨������ƽ̨�Ż��и���ƫ��
			DeltaLocToPrePlat = ForwardDir * PrePlatform->GetPlatformLength() * LengthScale;

		else DeltaLocToPrePlat = FVector::ZeroVector;
	}
	return TempTrans;
}

FTransform AMyPlayerController::GetSpawnTransf_Beam(ARunPlatform* PrePlatform)
{
	FTransform TempTrans;
	if (PrePlatform != nullptr)
	{
		AbsoluteDir = PrePlatform->PlatDir;    //��ƽֻ̨���������ǰ���ƽ̨��ǰ��
		FVector CurLocation = PrePlatform->SpawnLocation;
		FRotator CurRotation = PrePlatform->GetActorRotation();

		FMatrix RotatMatrix = FRotationMatrix(CurRotation);
		FVector ForwardDir = -RotatMatrix.GetUnitAxis(EAxis::X);

		const FVector DelLoc = ForwardDir * PrePlatform->GetPlatformLength() * 3;
		FVector SpawnLocation = CurLocation + DelLoc;
		TempTrans = FTransform(CurRotation, SpawnLocation);

		DeltaLocToPrePlat = DelLoc;
	}
	return TempTrans;
}

FTransform AMyPlayerController::GetSpawnTransf_Physic(ARunPlatform* PrePlatform)
{
	FTransform TempTrans;
	if (PrePlatform != nullptr)
	{
		AbsoluteDir = PrePlatform->PlatDir;    //��ƽֻ̨���������ǰ���ƽ̨��ǰ��
		FVector CurLocation = PrePlatform->SpawnLocation;
		FRotator CurRotation = PrePlatform->GetActorRotation();

		FMatrix RotatMatrix = FRotationMatrix(CurRotation);
		FVector ForwardDir = -RotatMatrix.GetUnitAxis(EAxis::X);

		const FVector DelLoc = ForwardDir * (PrePlatform->GetPlatformLength() + 200.f);
		FVector SpawnLocation = CurLocation + DelLoc;
		TempTrans = FTransform(CurRotation, SpawnLocation);

		DeltaLocToPrePlat = DelLoc;
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
			if (CurPlatformIndex >= 0 && CurPlatformIndex < PlatformArray.Num())
				if (CurPlatform->PlatDir == PlatformArray[CurPlatformIndex - 1]->PlatDir)
					BonusNum = CurPlatform->GetPlatformLength() / 100.f;
				else
					BonusNum = CurPlatform->GetPlatformLength() / 100.f - 1;
		}

		for (int32 i = 0; i < BonusNum; i++)
		{
			ABonus* SpawnBonus = GetWorld()->SpawnActorDeferred<ABonus>(Bonus_Score, FTransform(CurPlatform->GetActorRotation(), FirstSpawnLoc + 100 * i * SpawnDirX));
			if (SpawnBonus != nullptr)
			{
				SpawnBonus->RotateStartTime = 0.1f*i;
				UGameplayStatics::FinishSpawningActor(SpawnBonus, FTransform(CurPlatform->GetActorRotation(), FirstSpawnLoc + 100 * i * SpawnDirX));
			}
			
			SpawnBonus->AttachToActor(CurPlatform, FAttachmentTransformRules::KeepWorldTransform);
			CurPlatform->OnDestory.AddUObject(SpawnBonus, &ABonus::DestroyActor);
			CurPlatform->OnFall.AddUObject(SpawnBonus, &ABonus::StartFall);
		}
	}
}

void AMyPlayerController::SpawnBonus_NoObstacle(ARunPlatform* AttachedPlatform)
{
	int32 Random_Bonus_NoObs = FMath::Rand() % 100;
	ABonus* SpawnedNoObsBonus = nullptr;

	if (Random_Bonus_NoObs < 50)
	{
		FVector PlatformDir_X = FRotationMatrix(AttachedPlatform->GetActorRotation()).GetUnitAxis(EAxis::X);
		FVector PlatformDir_Y = FRotationMatrix(AttachedPlatform->GetActorRotation()).GetUnitAxis(EAxis::Y);
		FVector PlatformDir_Z = FRotationMatrix(AttachedPlatform->GetActorRotation()).GetUnitAxis(EAxis::Z);

		if (AttachedPlatform->IsA(SpawnPlatform_Beam))
		{
			ARunPlatform* const PrePlatform = PlatformArray[PlatformArray.Num() - 2];
			if (PrePlatform)
			{
				FVector PrePlatformDir_X = FRotationMatrix(PrePlatform->GetActorRotation()).GetUnitAxis(EAxis::X);
				FVector PrePlatformDir_Y = FRotationMatrix(PrePlatform->GetActorRotation()).GetUnitAxis(EAxis::Y);
				FVector PrePlatformDir_Z = FRotationMatrix(PrePlatform->GetActorRotation()).GetUnitAxis(EAxis::Z);
				float SpawnDistanceToPre = -700.f + (FMath::Rand() % 14) * 100.f;

				FVector SpawnLocation = PrePlatform->GetActorLocation() + PrePlatformDir_X * PrePlatform->GetPlatformLength() + PrePlatformDir_Y * (SpawnDistanceToPre + PrePlatform->GetPlatformWidth() / 2) + 200.f * PrePlatformDir_Z;
				ABonus* const BeamSpawnedNoObsBonus = GetWorld()->SpawnActor<ABonus>(Bonus_NoObstacle, FTransform(PrePlatform->GetActorRotation(), SpawnLocation));
				if(BeamSpawnedNoObsBonus)
					BeamSpawnedNoObsBonus->AttachToActor(PrePlatform, FAttachmentTransformRules::KeepWorldTransform);
			}
		}
		else if (AttachedPlatform->IsA(SpawnPlatform_Physic))
		{
			FVector SpawnLocation = AttachedPlatform->GetActorLocation() + (-PlatformDir_X) * AttachedPlatform->GetPlatformLength() * 2.f + PlatformDir_Y * AttachedPlatform->GetPlatformWidth() / 2 + 500.f * PlatformDir_Z;
			SpawnedNoObsBonus = GetWorld()->SpawnActor<ABonus>(Bonus_NoObstacle, FTransform(AttachedPlatform->GetActorRotation(), SpawnLocation));
		}
		else
		{
			FVector SpawnLocation = AttachedPlatform->GetActorLocation() + PlatformDir_X * AttachedPlatform->GetPlatformLength()/2 + PlatformDir_Y * AttachedPlatform->GetPlatformWidth() / 2 + 200.f * PlatformDir_Z;
			SpawnedNoObsBonus = GetWorld()->SpawnActor<ABonus>(Bonus_NoObstacle, FTransform(AttachedPlatform->GetActorRotation(), SpawnLocation));
		}

		if (SpawnedNoObsBonus != nullptr)
			SpawnedNoObsBonus->AttachToActor(AttachedPlatform, FAttachmentTransformRules::KeepWorldTransform);
	}
}

void AMyPlayerController::ChangeWeaponType(EWeaponType::Type WeaponType)
{
	if (CurrentWeaponType == EWeaponType::Weapon_Beam && InConnectedToPlat && CurConnectedPlat != nullptr)
	{
		CurConnectedPlat->DeActiveBeam();
		CurConnectedPlat = nullptr;
	}

	//��������Beam�ӵ�
	for (TActorIterator<ABullet> It(GetWorld()); It; ++It)
	{
		if ((*It)->CurWeaponType == EWeaponType::Weapon_Beam)
			(*It)->Destroy();
	}

	CurrentWeaponType = WeaponType;
}

void AMyPlayerController::RandomSpawnFlyObstacle()
{
	int32 CurNoShootPlatNum = 0;         //��ǰ��Shootƽ̨����Ŀ
	int32 PlatNum = PlatformArray.Num();
	int32 RandomFlyObstacle = FMath::Rand() % 100;

	for (int32 i = 0; i < PlatNum; ++i)
	{ 
		if (!PlatformArray[i]->IsA(SpawnPlatform_Shoot))
			CurNoShootPlatNum++;    //��0��ʼ��Shootƽ̨����Ŀ
		else
			break;
	}

	if (((CurNoShootPlatNum >= 7 && FlyObstacleSpawnInterval > 10) || FlyObstacleSpawnInterval == -1) && FlyObstacleArray.Num() < MaxFlyObstacles && RandomFlyObstacle < 40)    //40%�ļ������ɷ����ϰ�
	{
		if (SpawnFlyObstacle != nullptr)
		{
			if (CurNoShootPlatNum < PlatNum && PlatformArray[CurNoShootPlatNum]->CurBoundFlyObstacleNum < MaxFlyObstacles)    
			{
				AFlyObstacle* Obstacle = GetWorld()->SpawnActorDeferred<AFlyObstacle>(SpawnFlyObstacle, FTransform(PlatformArray[CurNoShootPlatNum - 1]->GetActorRotation(), PlatformArray[CurNoShootPlatNum - 1]->SpawnLocation + FVector(0.f, 0.f, 80.f)));
				if (Obstacle != nullptr)
				{
					Obstacle->AimCharacter = Cast<AMyFirstGameCharacter>(GetPawn());
					UGameplayStatics::FinishSpawningActor(Obstacle, FTransform(PlatformArray[CurNoShootPlatNum - 1]->GetActorRotation(), PlatformArray[CurNoShootPlatNum - 1]->SpawnLocation + FVector(0.f, 0.f, 80.f)));

					FlyObstacleSpawnInterval = 0;   //��ʼ�ۼӼ����ƽ̨
					FlyObstacleArray.Add(Obstacle);

					int32 CurObstacleNum = FlyObstacleArray.Num();
					for (int32 i = 0; i < CurObstacleNum; ++i)
					{
						AFlyObstacle* const CurObstacle = FlyObstacleArray[i];
						if (CurObstacle != nullptr)
						{
							PlatformArray[CurNoShootPlatNum]->FlyObstacleDestory.AddUObject(CurObstacle, &AFlyObstacle::StartDestroy);      //��ʼ��
							PlatformArray[CurNoShootPlatNum]->CurBoundFlyObstacleNum++;
						}
						GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Black, TEXT("�Ѱ�"));
					}
					FlyObstacleArray.Reset();   //�������
				}
			}
			else if (CurNoShootPlatNum == PlatNum)    //�����û��Shootƽ̨����ֱ������һ��FlyObstacle
			{
				AFlyObstacle* Obstacle = GetWorld()->SpawnActorDeferred<AFlyObstacle>(SpawnFlyObstacle, FTransform(PlatformArray[CurNoShootPlatNum - 1]->GetActorRotation(), PlatformArray[CurNoShootPlatNum - 1]->SpawnLocation + FVector(0.f, 0.f, 80.f)));
				if (Obstacle != nullptr)
				{
					Obstacle->AimCharacter = Cast<AMyFirstGameCharacter>(GetPawn());
					UGameplayStatics::FinishSpawningActor(Obstacle, FTransform(PlatformArray[CurNoShootPlatNum - 1]->GetActorRotation(), PlatformArray[CurNoShootPlatNum - 1]->SpawnLocation + FVector(0.f, 0.f, 80.f)));

					FlyObstacleSpawnInterval = 0;   //��ʼ�ۼӼ����ƽ̨
					FlyObstacleArray.Add(Obstacle);
				}
			}
		}
	}
	else if (FlyObstacleSpawnInterval != -1)
	{
		FlyObstacleSpawnInterval++;
	}
	
	if (PlatformArray.Last()->IsA(SpawnPlatform_Shoot))
	{
		int32 CurObstacleNum = FlyObstacleArray.Num();
		for (int32 i = 0; i < CurObstacleNum; ++i)
		{
			AFlyObstacle* const CurObstacle = FlyObstacleArray[i];
			if (CurObstacle != nullptr)
			{
				PlatformArray[CurNoShootPlatNum]->FlyObstacleDestory.AddUObject(CurObstacle, &AFlyObstacle::StartDestroy);       //��ʼ��
				PlatformArray[CurNoShootPlatNum]->CurBoundFlyObstacleNum++;
				GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Black, TEXT("�Ѱ�"));
			}
		}
		FlyObstacleArray.Reset();   //�������
	}
}

void AMyPlayerController::AddMaxSpawnObstacles()
{
	CurSpawnedShootPlats++;
	if (CurSpawnedShootPlats >= (MaxFlyObstacles * 10) && MaxFlyObstacles <= 4)      //��������4�������ϰ���̫�߻ᵼ����Ϸ�Ѷȸ�
	{
		MaxFlyObstacles++;
	}
}

void AMyPlayerController::TogglePauseStat()

{
	IsInPause = !IsInPause;

	if (IsInPause)
	{
		if (StopGameDelegate.IsBound())
			StopGameDelegate.Broadcast();

		this->SetPause(true);
	}
	else
	{
		this->SetPause(false);
	}
}

void AMyPlayerController::QuitGame()
{
	this->ConsoleCommand("quit");
}

void AMyPlayerController::StartToAll(int32 LastTime)
{
	IsToAll = true;

	int32 ArrayNum = PlatformArray.Num();

	ARunPlatform* ToAllStartPlat = nullptr;

	for (int32 i = 0; i < ArrayNum; ++i)
	{
		if (PlatformArray[i] != nullptr)
		{
			ToAllStartPlat = PlatformArray[i];
			break;
		}
	}
	if (ToAllStartPlat != nullptr)
	{
		if (ToAllStartPlat->NextPlatform)
		{
			ToAllStartPlat->MoveToAllFun(FVector::ZeroVector);
		}
	}

	//���������Ѵ��ڵķ����ϰ�
	for (TActorIterator<AFlyObstacle> It(GetWorld()); It; ++It)
	{
		if (*It)
			(*It)->StartDestroy();
	}

	GetWorldTimerManager().SetTimer(NoObstacleTime, this, &AMyPlayerController::ToStopToAllState, LastTime, false);
}

void AMyPlayerController::StartToAllTest()
{
	IsToAll = true;

	int32 ArrayNum = PlatformArray.Num();

	ARunPlatform* ToAllStartPlat = nullptr;

	for (int32 i = 0; i < ArrayNum; ++i)
	{
		if (PlatformArray[i] != nullptr)
		{
			ToAllStartPlat = PlatformArray[i];
			break;
		}
	}
	if (ToAllStartPlat != nullptr)
	{
		if (ToAllStartPlat->NextPlatform)
		{
			ToAllStartPlat->MoveToAllFun(FVector::ZeroVector);
		}
	}
}

void AMyPlayerController::ToStopToAllState()
{
	//һ����ִ��ʵ�ʲ����������Ѿ���ʼ����״̬
	GetWorldTimerManager().SetTimer(NoObstacleTime, this, &AMyPlayerController::StopToAll, 1, false);
	IsInStopToAllAnim = true;

	//��ʱҪ���������Ծ
	AMyFirstGameCharacter* MC = Cast<AMyFirstGameCharacter>(GetPawn());
	if (MC)
	{
		MC->GetCharacterMovement()->JumpZVelocity = 0.f;
	}

	ARunGameState* RGS = Cast<ARunGameState>(GetWorld()->GetGameState());
	if (RGS)
	{
		if (RGS->RemindDelegate.IsBound())
			RGS->RemindDelegate.Broadcast();
	}
}

void AMyPlayerController::StopToAll()
{
	int32 ArrayNum = PlatformArray.Num();
	
	IsToAll = false;
	ARunPlatform* ToOriginStartPlat = nullptr;
	
	for (int32 i = 0; i < ArrayNum; ++i)
	{
		if (PlatformArray[i] != nullptr)
		{
			ToOriginStartPlat = PlatformArray[i];
			break;
		}
	}

	if (ToOriginStartPlat != nullptr)
	{
		if (ToOriginStartPlat->IsA(SpawnPlatform_Beam) || ToOriginStartPlat->IsA(SpawnPlatform_Shoot))
		{
			if (ToOriginStartPlat->NextPlatform)
				ToOriginStartPlat->NextPlatform->StopToAllFun(FVector::ZeroVector);
		}
		else if (ToOriginStartPlat->IsA(SpawnPlatform_Physic))
		{
			if (!ToOriginStartPlat->Platform->IsSimulatingPhysics())
				ToOriginStartPlat->Platform->SetSimulatePhysics(true);

			ToOriginStartPlat->StopToAllFun(FVector::ZeroVector);
		}
		else
			ToOriginStartPlat->StopToAllFun(FVector::ZeroVector);
	}

	//Ԥ����������������
	GetWorldTimerManager().SetTimer(NoObstacleTime, this, &AMyPlayerController::StopToAllAnimEnd, 1.5f, false);
}

void AMyPlayerController::StopToAllAnimEnd()
{
	IsInStopToAllAnim = false;

	//�����Ҫ�ָ������Ծ
	AMyFirstGameCharacter* MC = Cast<AMyFirstGameCharacter>(GetPawn());
	if (MC)
	{
		MC->GetCharacterMovement()->JumpZVelocity = 500.f;
	}
}

void AMyPlayerController::NewSpawnedPlatformToAll(ARunPlatform* NewPlatformRef)
{
	ARunPlatform* const FrontLastPlat = PlatformArray[PlatformArray.Num() - 2];
	if (!FrontLastPlat->MoveToAll)     //��ƽ̨�����ƶ�
	{
		if (NewPlatformRef->IsA(SpawnPlatform) && (FrontLastPlat->IsA(SpawnPlatform) || FrontLastPlat->IsA(SpawnPlatform_Shoot)))      //����һ��ƽ̨����ͨƽ̨�����ҵ�ǰƽ̨����ͨƽ̨�����ƽ̨
		{
			NewPlatformRef->MoveToAllFun(FVector::ZeroVector);
		}
		else
		{
			const FVector NextDeltaPos = FrontLastPlat->SpawnLocation - (NewPlatformRef->GetActorLocation() + NewPlatformRef->GetPlatformLength() * FrontLastPlat->GetActorRotation().Vector());
			NewPlatformRef->MoveToAllFun(NextDeltaPos);
		}
	}
}