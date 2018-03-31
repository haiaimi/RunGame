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
	MaxFlyObstacles = 1;     //游戏开始时飞行障碍数为0
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
	PlatformArray.SetNum(10);       //平台数组的容量为10
	FlyObstacleArray.SetNum(0);

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
				//GEngine->AddOnScreenDebugMessage(1, 5, FColor::Black, TEXT("检索成功"));
				break;
			}
		}
	}

	//下面是逐步生成10个平台
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
			if (TempPlatform != CurPlatform && !PlatformArray.Last()->MoveToNew && CurPlatform != nullptr)   //玩家所在平台发生变化，并且最后一个平台不在移动时
			{
				int32 CurPlatIndex = PlatformArray.Find(CurPlatform);     //当前所在平台在数组中的位置
				
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
					float FallDistance = LastPlatformRef->GetActorLocation().Z - MC->GetActorLocation().Z;      //玩家下落距离，超过一定距离则认为游戏结束
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
	ARunPlatform* AddPlatform = nullptr;   //指向即将生成的平台

	for (int32 i = 0; i < SpawnNum; ++i)
	{
		int32 Random_Shoot = FMath::Rand() % 100;  //生成随机数，用来决定是否生成触发平台
		int32 Random_Beam = FMath::Rand() % 100;    //生成随机数，决定是否生成需要闪电枪触发的平台
		int32 Random_Physic = FMath::Rand() % 100;   //生成随机数，用来决定是否生成物理平台
		int32 Random_Bonus_Score = FMath::Rand() % 100;

		//下面就是随机生成部分，优先级是Beam > Physic > Shoot > Normal(正常平台)
		if (Random_Beam >= 0 && Random_Beam < 10 && PlatformArray.Last()->IsA(SpawnPlatform)/*!Cast<ARunPlatform_Shoot>(PlatformArray.Last()) && !Cast<ARunPlatform_Beam>(PlatformArray.Last())*/)  //5%的几率生成闪电平台，并且上一个平台不是射击触发型和闪电类型
		{
			
			AddPlatform = GetWorld()->SpawnActor<ARunPlatform_Beam>(SpawnPlatform_Beam, GetSpawnTransf_Beam(PlatformArray.Last()));
			PlatformArray.Last()->NoPlayerToSlope = true;   //该平台的上一个平台不进行旋转
			PlatformArray.Last()->SlopeAngle = 0.f;
		}
		else
		{
			if (Random_Physic >= 0 && Random_Physic < 10 && PlatformArray.Last()->IsA(SpawnPlatform))     //10%的几率生成物理平台
				AddPlatform = GetWorld()->SpawnActor<ARunPlatform_Physic>(SpawnPlatform_Physic, GetSpawnTransf_Physic(PlatformArray.Last()));
			else
			{
				if (Random_Shoot >= 0 && Random_Shoot < 10 && PlatformArray.Last()->IsA(SpawnPlatform)/*!Cast <ARunPlatform_Shoot>(PlatformArray.Last()) && !Cast<ARunPlatform_Beam>(PlatformArray.Last())*/)   //25%的几率生成触发型平台,并且前一个平台不是射击型和闪电型
					AddPlatform = GetWorld()->SpawnActor<ARunPlatform_Shoot>(SpawnPlatform_Shoot, GetSpawnTransf_Shoot(PlatformArray.Last()));
				else
					AddPlatform = GetWorld()->SpawnActor<ARunPlatform>(SpawnPlatform, GetRandomSpawnTransf(PlatformArray.Last()));
			}
		}

		if (AddPlatform != nullptr)
		{
			AddPlatform->PlatDir = AbsoluteDir;
			PlatformArray.Last()->NextPlatform = AddPlatform;  //指定下一个平台
			PlatformArray.Push(AddPlatform);

			if (PlatformArray[0] != nullptr)
				if (!PlatformArray[0]->IsInDestroyed)
					PlatformArray[0]->StartDestroy();  //开始删除第一个平台
			
			PlatformArray[0] = nullptr;
			PlatformArray.RemoveAt(0);  //移除已经走过的平台
			TempPlatform = CurPlatform;   //
			if (IsToAll)
				NewSpawnedPlatformToAll(AddPlatform);

			if (Random_Bonus_Score >= 0 && Random_Bonus_Score < 30)
				SpawnBonus_Score(AddPlatform);   //30的几率生成Bonus Score

			if (AddPlatform->IsA(SpawnPlatform_Shoot))
				AddMaxSpawnObstacles();

			AddPlatform->DeltaLocToPrePlat = DeltaLocToPrePlat;

			if (!IsToAll)
			{
				//RandomSpawnFlyObstacle();    //只有在非无障碍模式下才随机生成飞行障碍
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
		AbsoluteDir = PrePlatform->PlatDir;    //该平台只生成在相对前面的平台的前方
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

		uint8 Dir = 0;    //这是相对方向
		
		if(PreDir == EPlatformDirection::Absolute_Forward)
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
			
		int32 LengthScale = 1;    //控制生成的位置
		if (PrePlatform->IsA(SpawnPlatform_Beam) || PrePlatform->IsA(SpawnPlatform_Physic))   //如果前一个平台是闪电平台
		{
			Dir = 1;  //只生成在前一个方块的前方
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

		/**下面就是得出下一个生成平台的绝对世界方向*/
		if (PrePlatform->PlatDir == EPlatformDirection::Absolute_Forward)
			AbsoluteDir = (EPlatformDirection::Type)Dir;

		else if (PrePlatform->PlatDir == EPlatformDirection::Absolute_Left)
			AbsoluteDir = (Dir == 1) ? EPlatformDirection::Absolute_Left : EPlatformDirection::Absolute_Forward;

		else if (PrePlatform->PlatDir == EPlatformDirection::Absolute_Right)
			AbsoluteDir = (Dir == 0) ? EPlatformDirection::Absolute_Forward : EPlatformDirection::Absolute_Right;

		if (PrePlatform->IsA(SpawnPlatform_Physic))      //只有前一个平台是物理平台才会有附加偏移
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
		AbsoluteDir = PrePlatform->PlatDir;    //该平台只生成在相对前面的平台的前方
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
		AbsoluteDir = PrePlatform->PlatDir;    //该平台只生成在相对前面的平台的前方
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
		FMatrix Orient = FRotationMatrix(CurPlatform->GetActorRotation());    //平台方向
		FVector SpawnDirX = Orient.GetUnitAxis(EAxis::X);
		FVector SpawnDirY = Orient.GetUnitAxis(EAxis::Y);
		FVector SpawnDirZ = Orient.GetUnitAxis(EAxis::Z);

		FVector FirstSpawnLoc;
		int32 ScorePosOnPlat = FMath::Rand() % 3;     //0为左，1为中，2为右

		//下面就是设置三个位置的Score
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

	//销毁所有Beam子弹
	for (TActorIterator<ABullet> It(GetWorld()); It; ++It)
	{
		if ((*It)->CurWeaponType == EWeaponType::Weapon_Beam)
			(*It)->Destroy();
	}

	CurrentWeaponType = WeaponType;
}

void AMyPlayerController::RandomSpawnFlyObstacle()
{
	int32 CurNoShootPlatNum = 0;         //当前非Shoot平台的数目
	int32 PlatNum = PlatformArray.Num();
	int32 RandomFlyObstacle = FMath::Rand() % 100;

	for (int32 i = 0; i < PlatNum; ++i)
	{ 
		if (!PlatformArray[i]->IsA(SpawnPlatform_Shoot))
			CurNoShootPlatNum++;    //从0开始非Shoot平台的数目
		else
			break;
	}

	if (((CurNoShootPlatNum >= 7 && FlyObstacleSpawnInterval > 10) || FlyObstacleSpawnInterval == -1) && FlyObstacleArray.Num() < MaxFlyObstacles && RandomFlyObstacle < 40)    //40%的几率生成飞行障碍
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

					FlyObstacleSpawnInterval = 0;   //开始累加间隔的平台
					FlyObstacleArray.Add(Obstacle);

					int32 CurObstacleNum = FlyObstacleArray.Num();
					for (int32 i = 0; i < CurObstacleNum; ++i)
					{
						AFlyObstacle* const CurObstacle = FlyObstacleArray[i];
						if (CurObstacle != nullptr)
						{
							PlatformArray[CurNoShootPlatNum]->FlyObstacleDestory.AddUObject(CurObstacle, &AFlyObstacle::StartDestroy);      //开始绑定
							PlatformArray[CurNoShootPlatNum]->CurBoundFlyObstacleNum++;
						}
						GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Black, TEXT("已绑定"));
					}
					FlyObstacleArray.Reset();   //清空数组
				}
			}
			else if (CurNoShootPlatNum == PlatNum)    //如果还没有Shoot平台，就直接生成一个FlyObstacle
			{
				AFlyObstacle* Obstacle = GetWorld()->SpawnActorDeferred<AFlyObstacle>(SpawnFlyObstacle, FTransform(PlatformArray[CurNoShootPlatNum - 1]->GetActorRotation(), PlatformArray[CurNoShootPlatNum - 1]->SpawnLocation + FVector(0.f, 0.f, 80.f)));
				if (Obstacle != nullptr)
				{
					Obstacle->AimCharacter = Cast<AMyFirstGameCharacter>(GetPawn());
					UGameplayStatics::FinishSpawningActor(Obstacle, FTransform(PlatformArray[CurNoShootPlatNum - 1]->GetActorRotation(), PlatformArray[CurNoShootPlatNum - 1]->SpawnLocation + FVector(0.f, 0.f, 80.f)));

					FlyObstacleSpawnInterval = 0;   //开始累加间隔的平台
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
				PlatformArray[CurNoShootPlatNum]->FlyObstacleDestory.AddUObject(CurObstacle, &AFlyObstacle::StartDestroy);       //开始绑定
				PlatformArray[CurNoShootPlatNum]->CurBoundFlyObstacleNum++;
				GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Black, TEXT("已绑定"));
			}
		}
		FlyObstacleArray.Reset();   //清空数组
	}
}

void AMyPlayerController::AddMaxSpawnObstacles()
{
	CurSpawnedShootPlats++;
	if (CurSpawnedShootPlats >= (MaxFlyObstacles * 10) && MaxFlyObstacles <= 4)      //最多可以有4个飞行障碍，太高会导致游戏难度高
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

	//销毁所有已存在的飞行障碍
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
	//一秒后才执行实际操作，但是已经开始进入状态
	GetWorldTimerManager().SetTimer(NoObstacleTime, this, &AMyPlayerController::StopToAll, 1, false);
	IsInStopToAllAnim = true;

	//此时要限制玩家跳跃
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

	//预估动画在两秒后结束
	GetWorldTimerManager().SetTimer(NoObstacleTime, this, &AMyPlayerController::StopToAllAnimEnd, 1.5f, false);
}

void AMyPlayerController::StopToAllAnimEnd()
{
	IsInStopToAllAnim = false;

	//下面就要恢复玩家跳跃
	AMyFirstGameCharacter* MC = Cast<AMyFirstGameCharacter>(GetPawn());
	if (MC)
	{
		MC->GetCharacterMovement()->JumpZVelocity = 500.f;
	}
}

void AMyPlayerController::NewSpawnedPlatformToAll(ARunPlatform* NewPlatformRef)
{
	ARunPlatform* const FrontLastPlat = PlatformArray[PlatformArray.Num() - 2];
	if (!FrontLastPlat->MoveToAll)     //该平台不在移动
	{
		if (NewPlatformRef->IsA(SpawnPlatform) && (FrontLastPlat->IsA(SpawnPlatform) || FrontLastPlat->IsA(SpawnPlatform_Shoot)))      //当下一个平台是普通平台，并且当前平台是普通平台或射击平台
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