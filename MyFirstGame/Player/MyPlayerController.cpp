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
#include "GameFramework/PlayerStart.h"
#include "RunGameSave.h"
#include "Engine/LocalPlayer.h"
#include "Engine/TextureRenderTarget2D.h"
#include "IImageWrapperModule.h"
#include "FileManager.h"
#include "Paths.h"
#include "FileHelper.h"
#include "MyHUD.h"
#include "Door.h"
#include "JumpPlatform.h"
#include "WorldCollision.h"
#include "Common/RunGameHelper.h"
#include "DynamicMesh.h"
#include "IImageWrapper.h"
#include "Array.h"

static const float ShootPlatformAngle = 30.f;
static const FString RSaveGameSlot("RSaveGameSlot");

AMyPlayerController::AMyPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PlayerCameraManagerClass = AMyPlayerCameraManager::StaticClass();
	static ConstructorHelpers::FClassFinder<ARunPlatform> FSpawnPlat(TEXT("/Game/Blueprint/RunPlatform_BP"));
	static ConstructorHelpers::FClassFinder<ARunPlatform> FSpawnPlat_Shoot(TEXT("/Game/Blueprint/RunPlatform_Shoot_BP"));
	static ConstructorHelpers::FClassFinder<ARunPlatform> FSpawnPlat_Beam(TEXT("/Game/Blueprint/RunPlatform_Beam_BP"));
	static ConstructorHelpers::FClassFinder<ARunPlatform> FSpawnPlat_Physic(TEXT("/Game/Blueprint/RunPlatform_Physic_BP"));
	static ConstructorHelpers::FClassFinder<ARunPlatform> FSpawnPlat_Jump(TEXT("/Game/Blueprint/JumpPlatform_BP"));
	static ConstructorHelpers::FClassFinder<ABonus> FBonus_Score(TEXT("/Game/Blueprint/Bonus/Bonus_Score_BP"));
	static ConstructorHelpers::FClassFinder<ABonus> FBonus_NoObstacle(TEXT("/Game/Blueprint/Bonus/Bonus_NoObstacle_BP"));
	static ConstructorHelpers::FClassFinder<AFlyObstacle> FFlyObstacle(TEXT("/Game/Blueprint/FlyObstacle_BP"));

	SpawnPlatform = FSpawnPlat.Class;
	SpawnPlatform_Shoot = FSpawnPlat_Shoot.Class;
	SpawnPlatform_Beam = FSpawnPlat_Beam.Class;
	SpawnPlatform_Physic = FSpawnPlat_Physic.Class;
	SpawnPlatform_Jump = FSpawnPlat_Jump.Class;
	Bonus_Score = FBonus_Score.Class;
	Bonus_NoObstacle = FBonus_NoObstacle.Class;
	SpawnFlyObstacle = FFlyObstacle.Class;
	
	CurrentWeaponType = EWeaponType::Weapon_Instant;
	InConnectedToPlat = false;
	CurConnectedPlat = nullptr;
	bSpawnedJumpPlat = false;
	FlyObstacleSpawnInterval = -1;
	MaxFlyObstacles = 1;     //游戏开始时飞行障碍数为0
	CurSpawnedShootPlats = 0;
	IsInPause = 0;
	SpawnNoObsBonusParam = 0;
	PlatformState = (uint32)0x000fffff;
}


void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction("TogglePause", IE_Pressed, this, &AMyPlayerController::TogglePauseStat);
	InputComponent->BindAction("TestToAll", IE_Pressed, this, &AMyPlayerController::StartToAllTest);
	InputComponent->BindAction("StopToAll", IE_Pressed, this, &AMyPlayerController::StopToAll);
	InputComponent->BindAction("ScreenShot", IE_Pressed, this, &AMyPlayerController::ScreenShoot);
}

void AMyPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	PlatformArray.SetNum(15);       //设置平台数组的容量
	FlyObstacleArray.SetNum(0);

	InitPlatforms();
	SaveGameSlot = RSaveGameSlot;
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	RunGameHelper::LoadAsset(this);
	//测试左移右移的补位情况
	/*uint32 a = 1;
	a = a << 10;
	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::FormatAsNumber(a));*/
}

void AMyPlayerController::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (PlatformArray.Num() > 0)
	{
		ARunPlatform* LastPlatformRef = PlatformArray.Last();
		if (LastPlatformRef != nullptr)
		{
			if (TempPlatform != CurPlatform && !PlatformArray.Last()->MoveToNew && CurPlatform != nullptr 
				&& !PlatformArray.Last()->MoveToOrigin && !PlatformArray.Last()->MoveToAll  //玩家所在平台发生变化，并且最后一个平台不在移动时
				&& !(PlatformArray.Last()->IsToAll && !IsToAll))  //平台处于归位动画下，不生成平台
			{
				int32 CurPlatIndex = PlatformArray.Find(CurPlatform);     //当前所在平台在数组中的位置
				
				ARunGameState* RGS = Cast<ARunGameState>(GetWorld()->GetGameState());

				if (RGS)
				{
					RGS->SetCaptureDir(CurPlatform->PlatDir);
					RGS->AddPlayerDistance((CurPlatform->GetActorLocation() - TempPlatform->GetActorLocation()).Size2D());

					if (CurPlatform->IsA(SpawnPlatform_Shoot))
					{
						RGS->AddPlayerHeight(CurPlatform->GetActorLocation().Z);
					}
				}
				//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::Printf(TEXT("Player Distance: %f, Template Lenght: %f, DeltaDistance: %f"), PlayerMoveDistance, TempPlatform->GetPlatformLength(), (CurPlatform->GetActorLocation() - TempPlatform->GetActorLocation()).Size2D()));
				
				RandomSpawnPlatform(CurPlatIndex);
			}

			if (!IsInPause)
			{
				AMyFirstGameCharacter* MC = Cast<AMyFirstGameCharacter>(GetPawn());
				if (MC)
				{
					int32 PlatNum = PlatformArray.Num();
					for (int32 i = 0; i < PlatNum; i++)
					{
						if (PlatformArray[i] != nullptr)
							if (PlatformArray[i]->IsInDestroyed)
							{
								PlatformArray[i] = nullptr;
							}
							else
							{
								float FallDistance = PlatformArray[i]->GetActorLocation().Z - MC->GetActorLocation().Z;      //玩家下落距离，超过一定距离则认为游戏结束
								if (FallDistance > 2000.f)
								{
									bIsGameEnd = true;
									TogglePauseStat();
									SaveGame();      //游戏结束，暂停并保存游戏数据
								}
									break;   //结束循环
							}
					}
				}
			}
		}
	}

}

void AMyPlayerController::Destroyed()
{
	Super::Destroyed();
}

void AMyPlayerController::InitPlatforms()
{
	CurPlatform = nullptr;
	//清空数组中的平台
	for (auto iter : PlatformArray)
	{
		if (iter != nullptr)
			iter->StartDestroy();
	}

	// 销毁所有已存在的飞行障碍
	for (TActorIterator<AFlyObstacle> It(GetWorld()); It; ++It)
	{
		if (*It)
			(*It)->StartDestroy();
	}
	FlyObstacleArray.Reset(0); //清空飞行障碍数组

	//获取游戏世界中预设的一个平台，（默认的一个）
	for (TActorIterator<ARunPlatform> It(GetWorld()); It; ++It)
	{
		if ((*It)->Tags.Num())
		{
			if ((*It)->ActorHasTag(FName("StartPlatform")) && !(*It)->IsInDestroyed)   //如果是指定的开始平台就开始
			{
				CurPlatform = *It;
				TempPlatform = CurPlatform;
				PlatformArray[0] = *It;  //数组的第一个就是默认平台
										
				break;
			}
		}
	}

	if (!CurPlatform)   //没有找到适合的平台就重新生成一个
	{
		ARunPlatform* Spawned = GetWorld()->SpawnActor<ARunPlatform>(SpawnPlatform, FVector(3310.f, 1330.f, 20.f), FRotator(0.f, 180.f, 0.f));
		if (Spawned)
		{
			CurPlatform = Spawned;
			TempPlatform = CurPlatform;
			PlatformArray[0] = Spawned;
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

void AMyPlayerController::RestartGame()
{
	//查找PlayerStart
	APlayerStart* Start = nullptr;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{	//使用第一个找到的Start
		if (*It)
		{
			Start = *It;
			break;
		}
	}
	
	for (TActorIterator<ADoor> It(GetWorld()); It; ++It)
	{
		if (*It && (*It)->ActorHasTag(FName("StartPoint")))
		{
			UE_LOG(LogRunGame, Log, TEXT("Reset Door"))
			(*It)->bReset = true;      //重置门的状态
		}
	}

	for (TActorIterator<ARunPlatform> It(GetWorld()); It; ++It)
	{
		if (*It && !(*It)->IsInDestroyed && (*It)->ActorHasTag(TEXT("JumpPlatform")))
			(*It)->StartDestroy();
	}

	if (Start && GetPawn())
	{
		AMyFirstGameCharacter* MC = Cast<AMyFirstGameCharacter>(GetPawn());
		MC->TeleportTo(Start->GetActorLocation(), Start->GetActorRotation());     //把玩家移到开始的位置
		MC->IsInAccelerate = false;
		MC->IsTargeting = false;

		//恢复玩家速度
		MC->IsInAccelerate = false;
		MC->CurMaxAcclerateSpeed = MaxAcclerateSpeed;
		MC->CurMaxRunSpeed = MaxRunSpeed;
		MC->AddedSpeed = 0.f;
		MC->GetCharacterMovement()->ClearAccumulatedForces();

		//如果玩家在下蹲状态就恢复站立状态
		if (MC->IsInCrounch)
			MC->ToggleCrounchStat();
		this->SetControlRotation(Start->GetActorRotation());

		if (CurrentWeaponType == EWeaponType::Type::Weapon_Beam)
		{
			MC->NextWeapon();
		}
	}

	InitPlatforms();
	if (!bIsGameEnd)
		SaveGame(); //保存本次游戏得分

	GetWorldTimerManager().ClearTimer(NoObstacleTime);
	GetWorldTimerManager().ValidateHandle(NoObstacleTime);  //重新激活计时器
	IsToAll = false;
	CurSpawnedShootPlats = 0; //重置已生成平台数目
	MaxFlyObstacles = 0;
	bIsGameEnd = false;    //游戏已重启
	PlatformState = (uint32)0x000fffff;
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
		int32 Random_Jump = FMath::Rand() % 100;    //生成跳跃平台的

		//测试随机种子
		/*FRandomStream Stream(1);
		int32 RandTest = Stream.RandRange(0, 100);
		GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Black, FString::FormatAsNumber(RandTest));*/

		//这是在处于所有平台恢复原位时的过渡，此时不生成Beam
		if (PlatformArray.Last()->IsToAll && !IsToAll)
			Random_Beam = -1;    //不生成Beam平台

		//下面就是随机生成部分，优先级是Jump > Beam > Physic > Shoot > Normal(正常平台)
		if (Random_Jump >= 0 && Random_Jump < 50 && PlatformState == 0 && !bSpawnedJumpPlat && !IsToAll)
		{
			SpawnJumpPlatform(PlatformArray.Last());
		}
		else
		{
			if (Random_Beam >= 0 && Random_Beam < 8 && PlatformArray.Last()->IsA(SpawnPlatform) && !bSpawnedJumpPlat/*!Cast<ARunPlatform_Shoot>(PlatformArray.Last()) && !Cast<ARunPlatform_Beam>(PlatformArray.Last())*/)  //5%的几率生成闪电平台，并且上一个平台不是射击触发型和闪电类型
			{

				AddPlatform = GetWorld()->SpawnActor<ARunPlatform_Beam>(SpawnPlatform_Beam, GetSpawnTransf_Beam(PlatformArray.Last()));
				PlatformArray.Last()->NoPlayerToSlope = true;   //该平台的上一个平台不进行旋转
				PlatformArray.Last()->SlopeAngle = 0.f;
			}
			else
			{
				if (Random_Physic >= 0 && Random_Physic < 10 && PlatformArray.Last()->IsA(SpawnPlatform) && !bSpawnedJumpPlat)     //10%的几率生成物理平台
					AddPlatform = GetWorld()->SpawnActor<ARunPlatform_Physic>(SpawnPlatform_Physic, GetSpawnTransf_Physic(PlatformArray.Last()));
				else
				{
					if (Random_Shoot >= 0 && Random_Shoot < 10 && PlatformArray.Last()->IsA(SpawnPlatform) && !bSpawnedJumpPlat/*!Cast <ARunPlatform_Shoot>(PlatformArray.Last()) && !Cast<ARunPlatform_Beam>(PlatformArray.Last())*/)   //25%的几率生成触发型平台,并且前一个平台不是射击型和闪电型
						AddPlatform = GetWorld()->SpawnActor<ARunPlatform_Shoot>(SpawnPlatform_Shoot, GetSpawnTransf_Shoot(PlatformArray.Last()));
					else
						AddPlatform = GetWorld()->SpawnActor<ARunPlatform>(SpawnPlatform, GetRandomSpawnTransf(PlatformArray.Last()));
				}
			}

			if (AddPlatform != nullptr)
			{
				AddPlatform->PlatDir = AbsoluteDir;
				PlatformArray.Last()->NextPlatform = AddPlatform;  //指定下一个平台

				if (PlatformArray[0] != nullptr)
					if (!PlatformArray[0]->IsInDestroyed)
						PlatformArray[0]->StartDestroy();  //开始删除第一个平台

				PlatformArray[0] = nullptr;
				PlatformArray.RemoveAt(0);  //移除已经走过的平台
				PlatformState = PlatformState >> 1;    //1号位已经清除
				TempPlatform = CurPlatform;   //
				if (PlatformArray.Last()->IsToAll || IsToAll)
					NewSpawnedPlatformToAll(AddPlatform);

				PlatformArray.Push(AddPlatform);
				if (Random_Bonus_Score >= 0 && Random_Bonus_Score < 30)
					SpawnBonus_Score(AddPlatform);   //30的几率生成Bonus Score

				if (AddPlatform->IsA(SpawnPlatform_Shoot))
					AddMaxSpawnObstacles();

				if (AddPlatform->IsA(SpawnPlatform_Beam))
				{
					//如果是闪电平台则第20位及平台数最后一个为1
					uint32 PlatNum = PlatformArray.Num();
					PlatformState |= (uint32)1 << (PlatNum - 1);
				}

				AddPlatform->DeltaLocToPrePlat = DeltaLocToPrePlat;

				if (!IsToAll)
				{
					RandomSpawnFlyObstacle();    //只有在非无障碍模式下才随机生成飞行障碍
					SpawnNoObsBonusParam++;

					if (SpawnNoObsBonusParam >= PlatformArray.Num() * 3)
					{
						SpawnBonus_NoObstacle(AddPlatform);
						if (!AddPlatform->IsA(SpawnPlatform_Beam))
						{
							uint32 PlatNum = PlatformArray.Num();
							PlatformState |= (uint32)1 << (PlatNum - 1);
						}
						SpawnNoObsBonusParam = 0;
					}
				}
				UE_LOG(LogRunGame, Log, TEXT("%d"), PlatformState)

					if ((AddPlatform->MoveToAll || AddPlatform->MoveToOrigin) && AddPlatform->DeltaLoc.Size() > 1.f)
						break;

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
		FVector CurLocation;
		if (bSpawnedJumpPlat)
		{
			CurLocation = PrePlatform->SpawnLocation + DeltaLocToPrePlat;
			bSpawnedJumpPlat = false;
		}
		else
			CurLocation = PrePlatform->SpawnLocation;

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
	int32 CurPlatformIndex, BonusNum = 0;

	if (PlatformArray.Find(CurPlatform, CurPlatformIndex))
	{
		RunGameHelper::ArrangeCoins(GetWorld(), Bonus_Score, CurPlatform, PlatformArray[CurPlatformIndex - 1]->PlatDir);
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

		FActorSpawnParameters SpawnParameter;
		SpawnParameter.Owner = this;
		
		if (AttachedPlatform->IsA(SpawnPlatform_Beam))
		{
			ARunPlatform* const PrePlatform = PlatformArray[PlatformArray.Num() - 2];
			if (PrePlatform)
			{
				FVector PrePlatformDir_X = -FRotationMatrix(PrePlatform->GetActorRotation()).GetUnitAxis(EAxis::X);
				FVector PrePlatformDir_Y = FRotationMatrix(PrePlatform->GetActorRotation()).GetUnitAxis(EAxis::Y);
				FVector PrePlatformDir_Z = FRotationMatrix(PrePlatform->GetActorRotation()).GetUnitAxis(EAxis::Z);
				float SpawnDistanceToPre = -700.f + (FMath::Rand() % 14) * 100.f;

				FVector SpawnLocation = PrePlatform->GetActorLocation() + PrePlatformDir_X * PrePlatform->GetPlatformLength() + PrePlatformDir_Y * (SpawnDistanceToPre + PrePlatform->GetPlatformWidth() / 2) + 200.f * PrePlatformDir_Z;
				ABonus* const BeamSpawnedNoObsBonus = GetWorld()->SpawnActor<ABonus>(Bonus_NoObstacle, FTransform(PrePlatform->GetActorRotation(), SpawnLocation), SpawnParameter);
				if (BeamSpawnedNoObsBonus != nullptr)
				{
					BeamSpawnedNoObsBonus->AttachToActor(PrePlatform, FAttachmentTransformRules::KeepWorldTransform);
					AttachedPlatform->OnDestory.AddUObject(BeamSpawnedNoObsBonus, &ABonus::DestroyActor);
					AttachedPlatform->OnFall.AddUObject(BeamSpawnedNoObsBonus, &ABonus::StartFall);
				}
			}
		}
		else if (AttachedPlatform->IsA(SpawnPlatform_Physic))
		{
			FVector SpawnLocation = AttachedPlatform->GetActorLocation() + (-PlatformDir_X) * AttachedPlatform->GetPlatformLength() * 2.f + PlatformDir_Y * AttachedPlatform->GetPlatformWidth() / 2 + 500.f * PlatformDir_Z;
			SpawnedNoObsBonus = GetWorld()->SpawnActor<ABonus>(Bonus_NoObstacle, FTransform(AttachedPlatform->GetActorRotation(), SpawnLocation), SpawnParameter);
		}
		else
		{
			FVector SpawnLocation = AttachedPlatform->GetActorLocation() + PlatformDir_X * AttachedPlatform->GetPlatformLength()/2 + PlatformDir_Y * AttachedPlatform->GetPlatformWidth() / 2 + 200.f * PlatformDir_Z;
			SpawnedNoObsBonus = GetWorld()->SpawnActor<ABonus>(Bonus_NoObstacle, FTransform(AttachedPlatform->GetActorRotation(), SpawnLocation), SpawnParameter);
		}	

		if (SpawnedNoObsBonus != nullptr)
		{
			SpawnedNoObsBonus->AttachToActor(AttachedPlatform, FAttachmentTransformRules::KeepWorldTransform);
			AttachedPlatform->OnDestory.AddUObject(SpawnedNoObsBonus, &ABonus::DestroyActor);
			AttachedPlatform->OnFall.AddUObject(SpawnedNoObsBonus, &ABonus::StartFall);
		}
	}
}

void AMyPlayerController::SpawnJumpPlatform(ARunPlatform* PrePlatform)
{
	//下面生成8个跳跃平台
	TEnumAsByte<EPlatformDirection::Type> Dir = PrePlatform->PlatDir;
	FVector PreLocation = PrePlatform->SpawnLocation;
	FVector SpawnDir_X = -FRotationMatrix(PrePlatform->GetActorRotation()).GetUnitAxis(EAxis::X);
	FVector SpawnDir_Y = FRotationMatrix(PrePlatform->GetActorRotation()).GetUnitAxis(EAxis::Y);
	FRotator LeftRotation, RightRotation;
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Black, TEXT("测试生成JumpPlatform"));

	switch (Dir)
	{
	case EPlatformDirection::Type::Absolute_Forward:
		LeftRotation = FRotator(0.f, 180.f, -30.f);
		RightRotation = FRotator(0.f, 0.f, -30.f);
		break;

	case EPlatformDirection::Type::Absolute_Left:
		LeftRotation = FRotator(0.f, 90.f, -30.f);
		RightRotation = FRotator(0.f, -90.f, -30.f);
		break;

	case EPlatformDirection::Type::Absolute_Right:
		LeftRotation = FRotator(0.f, -90.f, -30.f);
		RightRotation = FRotator(0.f, 90.f, -30.f);
		break;

	default:
		break;
	}

	//下面是检测两边是否已生成平台形成障碍
	FHitResult resultLeft, resultRight;
	FCollisionObjectQueryParams ObjectQueryParams(ECollisionChannel::ECC_WorldDynamic);
	FCollisionQueryParams QueryParams(TEXT("JumpQuery"));
	GetWorld()->SweepSingleByObjectType(resultRight,
									    PreLocation + 400.f * SpawnDir_X - 300.f * SpawnDir_Y,
										PreLocation + 1000.f*SpawnDir_X,
										FQuat(LeftRotation),
										ObjectQueryParams,
										FCollisionShape::MakeBox(FVector(200.f,200.f,200.f)),
										QueryParams);

	GetWorld()->SweepSingleByObjectType(resultLeft,
										PreLocation + 400.f * SpawnDir_X + 700.f * SpawnDir_Y,
										PreLocation + 1000.f*SpawnDir_X + 400.f * SpawnDir_Y,
										FQuat(LeftRotation),
										ObjectQueryParams,
										FCollisionShape::MakeBox(FVector(200.f, 200.f, 200.f)),
										QueryParams);
	if (!Cast<ARunPlatform>(resultLeft.GetActor()) && !Cast<ARunPlatform>(resultRight.GetActor()))
	{
		int PlatPairs = FMath::Rand() % 4 + 1;       //生成的跳跃平台的对数为 1~4
		
		AJumpPlatform* LSpawnedPlat = nullptr;
		AJumpPlatform* RSpawnedPlat = nullptr;

		for (int32 i = 0; i < PlatPairs; i++)
		{
			//生成右边的跳跃平台
			FTransform SpawnTransform = FTransform(FRotator::ZeroRotator, PreLocation + SpawnDir_X * (i*2000.f + 200.f));
			RSpawnedPlat = GetWorld()->SpawnActorDeferred<AJumpPlatform>(SpawnPlatform_Jump, SpawnTransform);
			if (RSpawnedPlat)
			{
				RSpawnedPlat->SpawnRotation = RightRotation;
				RSpawnedPlat->PlatDir = Dir;
				RSpawnedPlat->MoveStartTime = i * 0.25f;
				RSpawnedPlat->Tags.Add(TEXT("JumpPlatform"));
				UGameplayStatics::FinishSpawningActor(RSpawnedPlat, SpawnTransform);
				if (LSpawnedPlat != nullptr)
				{
					LSpawnedPlat->NextPlatform = RSpawnedPlat;
					RSpawnedPlat->PrePlatform = LSpawnedPlat;
				}
			}
			//生成左边的
			SpawnTransform = FTransform(FRotator::ZeroRotator, PreLocation + SpawnDir_X * (i*2000.f + 1600.f) + 400.f * SpawnDir_Y);
			LSpawnedPlat = GetWorld()->SpawnActorDeferred<AJumpPlatform>(SpawnPlatform_Jump, SpawnTransform);
			if (LSpawnedPlat)
			{
				LSpawnedPlat->SpawnRotation = LeftRotation;
				LSpawnedPlat->PlatDir = Dir;
				LSpawnedPlat->MoveStartTime = i * 0.25f;
				LSpawnedPlat->Tags.Add(TEXT("JumpPlatform"));
				UGameplayStatics::FinishSpawningActor(LSpawnedPlat, SpawnTransform);
				if (RSpawnedPlat != nullptr)
				{
					RSpawnedPlat->NextPlatform = LSpawnedPlat;
					LSpawnedPlat->PrePlatform = RSpawnedPlat;
				}
			}
		}

		LSpawnedPlat = RSpawnedPlat = nullptr;
		PlatformState = MAX_uint32;
		bSpawnedJumpPlat = true;
		DeltaLocToPrePlat = (2000.f*(PlatPairs - 1) + 1800.f)*SpawnDir_X;
		PrePlatform->NoPlayerToSlope = true;    //前一个平台不倾斜，降低难度
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
		if (PlatformArray[i] != nullptr)
		{
			if (!PlatformArray[i]->IsA(SpawnPlatform_Shoot))
				CurNoShootPlatNum++;    //从0开始非Shoot平台的数目
			else
				break;
		}
	}

	if (((CurNoShootPlatNum >= 10 && FlyObstacleSpawnInterval > 10) || FlyObstacleSpawnInterval == -1) && FlyObstacleArray.Num() < MaxFlyObstacles && RandomFlyObstacle < 10)    //10%的几率生成飞行障碍
	{
		if (!ensure(*SpawnFlyObstacle))return;     //判断是否查询到FlyObstacle类

		if (CurNoShootPlatNum < PlatNum && PlatformArray[CurNoShootPlatNum] && PlatformArray[CurNoShootPlatNum]->CurBoundFlyObstacleNum < MaxFlyObstacles)
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
				//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Black, TEXT("已绑定"));
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
	AMyPlayerCameraManager* MPCM = Cast<AMyPlayerCameraManager>(PlayerCameraManager);
	AMyHUD* HUD = Cast<AMyHUD>(GetHUD());

	if (IsInPause)
	{
		if (StopGameDelegate.IsBound())
			StopGameDelegate.Broadcast();

		MPCM->StartGaussianUI();
		this->SetPause(true);
		
		if (HUD)
			HUD->bDrawCrosshair = false;        //游戏暂停状态不现实准心
	}
	else
	{
		MPCM->StopGaussianUI();
		this->SetPause(false);

		if (HUD)
			HUD->bDrawCrosshair = true;
	}
}

void AMyPlayerController::QuitGame()
{
	SaveGame();
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
	GetWorldTimerManager().SetTimer(NoObstacleTime, this, &AMyPlayerController::StopToAll, 2, false);
	IsInStopToAllAnim = true;

	//此时要限制玩家跳跃
	/*AMyFirstGameCharacter* MC = Cast<AMyFirstGameCharacter>(GetPawn());
	if (MC)
	{
		MC->GetCharacterMovement()->JumpZVelocity = 0.f;
	}*/

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
	
	if (CurPlatform != nullptr)
		ToOriginStartPlat = CurPlatform;
	else
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
	//GetWorldTimerManager().SetTimer(NoObstacleTime, this, &AMyPlayerController::StopToAllAnimEnd, 1.5f, false);
}

//void AMyPlayerController::StopToAllAnimEnd()
//{
//	IsInStopToAllAnim = false;
//
//	//下面就要恢复玩家跳跃
//	AMyFirstGameCharacter* MC = Cast<AMyFirstGameCharacter>(GetPawn());
//	if (MC)
//	{
//		MC->GetCharacterMovement()->JumpZVelocity = 500.f;
//	}
//}

void AMyPlayerController::NewSpawnedPlatformToAll(ARunPlatform* NewPlatformRef)
{
	ARunPlatform* const FrontLastPlat = PlatformArray.Last();
	
	if (NewPlatformRef->IsA(SpawnPlatform) && (FrontLastPlat->IsA(SpawnPlatform) || FrontLastPlat->IsA(SpawnPlatform_Shoot)))      //当下一个平台是普通平台，并且当前平台是普通平台或射击平台
	{
		NewPlatformRef->MoveToAllFun(FVector::ZeroVector);
	}
	else
	{
		const FVector NextDeltaPos = FrontLastPlat->SpawnLocation - (NewPlatformRef->GetActorLocation() + NewPlatformRef->GetPlatformLength() * FrontLastPlat->GetActorRotation().Vector());  //每个平台移动到固定位置不动时，才会生成下一个平台
		NewPlatformRef->MoveToAllFun(NextDeltaPos);
	}
}

void AMyPlayerController::SaveGame()
{
	ARunGameState* RGS = Cast<ARunGameState>(GetWorld()->GetGameState());

	URunGameSave* RunSaveGame = Cast<URunGameSave>(UGameplayStatics::LoadGameFromSlot(RSaveGameSlot, 0));
	
	if (RunSaveGame)   //加载成功
	{
		if (RGS)
		{
			float NewScore = RGS->PlayerScore;
			RunSaveGame->Scores.Add(NewScore);
			RunSaveGame->Scores.Sort();   //数组进行排序,注意是升序
			RunSaveGame->Scores.RemoveAt(0);     //移除
			UGameplayStatics::SaveGameToSlot(RunSaveGame, RSaveGameSlot, 0);  //保存玩家分数
		}
	}
	else  
	{
		URunGameSave* _RunSaveGame = Cast<URunGameSave>(UGameplayStatics::CreateSaveGameObject(URunGameSave::StaticClass()));
		if (_RunSaveGame)
		{
			_RunSaveGame->Scores.SetNumZeroed(10);
			if (RGS)
				_RunSaveGame->Scores[_RunSaveGame->Scores.Num() - 1] = RGS->PlayerScore;
		}
		UGameplayStatics::SaveGameToSlot(_RunSaveGame, RSaveGameSlot, 0);  //保存玩家分数
	}
	//清空当前分数
	if (RGS)
	{
		RGS->RestartGame();
		RGS->UpdatePlayerScore(0.f);
	}
}

void AMyPlayerController::ScreenShoot()
{
	FViewport* viewport = GetLocalPlayer()->ViewportClient->Viewport;
	
	//FViewport* viewport = GetWorld()->GetGameViewport()->Viewport;
	TArray<FColor> PixelBuffer;
	viewport->ReadPixels(PixelBuffer);

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
	ImageWrapper->SetRaw((void*)PixelBuffer.GetData(), PixelBuffer.GetAllocatedSize(), viewport->GetSizeXY().X, viewport->GetSizeXY().Y, ERGBFormat::BGRA, 8);      //设置JPEG格式图片数据

	if (ImageWrapper.IsValid())       //
	{
		TArray<uint8> ImageData = ImageWrapper->GetCompressed();
		FString ScreenShotDir = FPaths::ScreenShotDir() / TEXT("ScreenShot");
		IFileManager::Get().MakeDirectory(*FPaths::ScreenShotDir(), true);
		FString SavePath = ScreenShotDir + TEXT("000.jpg");
		int32 PictureIndex = 1;      //截图文件夹中图片序号
		while (IFileManager::Get().FileExists(*SavePath))
		{
			SavePath = ScreenShotDir;

			if (PictureIndex <= 99)
				SavePath.Append(TEXT("0"));
			if (PictureIndex <= 9)
				SavePath.Append(TEXT("0"));
			SavePath.AppendInt(PictureIndex);
			SavePath.Append(".jpg");

			PictureIndex++;
		}
		FFileHelper::SaveArrayToFile(ImageData, *SavePath);            //保存图片数据
	}
}