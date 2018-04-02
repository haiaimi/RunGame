// Fill out your copyright notice in the Description page of Project Settings.

#include "FlyObstacle.h"
#include "Components/StaticMeshComponent.h"
#include "MyFirstGameCharacter.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "RunPlatform.h"
#include "MyFirstGame.h"
#include "Player/MyPlayerController.h"
#include "RunGameState.h"


// Sets default values
AFlyObstacle::AFlyObstacle(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FlyObstacleMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("FlyObstacleMesh"));
	SceneRoot = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("FlyObstacleRoot"));

	RootComponent = SceneRoot;
	FlyObstacleMesh->SetupAttachment(RootComponent);
	FlyObstacleMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlyObstacleMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);         //只对玩家产生碰撞

	//默认初速度与加速度
	StartSpeed = CurSpeed = 0.f;
	AccelerateSpeed = 200.f;
	IsOver = false;
	ForceActive = false;
	StopLengthTime = 0.f;
	ShouldShowDir = true;
}

void AFlyObstacle::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (AimCharacter != nullptr)
	{
		FlyDir = (AimCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();         //设置初始的方向
		FlyDst = AimCharacter->GetActorLocation();
		QueryIsOverSubAngle();
	}

	HalfObstacleSize = FlyObstacleMesh->Bounds.BoxExtent;
	FlyObstacleMesh->SetRelativeLocation(FVector(-HalfObstacleSize.X, -HalfObstacleSize.Y, 0.f));
}

// Called when the game starts or when spawned
void AFlyObstacle::BeginPlay()
{
	Super::BeginPlay();

	ShowObstaclePosRelativeToChar();
	ToStopAccelerate = -AccelerateSpeed * 2;
//测试条件编译
#ifdef WITH_EDITOR
	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Black, TEXT("In Editor"));

#elif PLATFORM_DESKTOP
	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Black, TEXT("In Windows"));

#endif // WITH_EDITOR

}

// Called every frame
void AFlyObstacle::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (FVector::DotProduct(FlyDir, (GetActorLocation() - FlyDst).GetSafeNormal2D()) < 0)        //这是还未超过目标点的情况
	{
		SetActorLocation(GetActorLocation() + FlyDir * CurSpeed * DeltaTime);

		const FRotator ObstacleRotation = GetActorRotation();
		SetActorRotation(FRotator(ObstacleRotation.Pitch, ObstacleRotation.Yaw + CurSpeed * DeltaTime/2.f , ObstacleRotation.Roll));  //同时跟随移动速度旋转
		CurSpeed += AccelerateSpeed * DeltaTime;
		if (CurSpeed >= 2000.f)
			CurSpeed = 2000.f;

		ToCharMaxSpeed = CurSpeed;
	}
	else          //这是超过目标点的情况
	{
		if (CurSpeed > 0)
		{
			if (ToCharMaxSpeed == CurSpeed)
			{
				if (ToCharMaxSpeed < 1000.f)
				{
					CurSpeed = ToCharMaxSpeed = 1000.f;      //使其最小的减速初速度为500，防止停在平台上过长时间
				}
				SelectSuitStopAccelerate(FlyDir, ToCharMaxSpeed, ComputeDistanceToStop(ToCharMaxSpeed, ToStopAccelerate));
			}

			IsOver = true;
			SetActorLocation(GetActorLocation() + FlyDir * CurSpeed * DeltaTime);
			const FRotator ObstacleRotation = GetActorRotation();
			SetActorRotation(FRotator(ObstacleRotation.Pitch, ObstacleRotation.Yaw + CurSpeed * DeltaTime, ObstacleRotation.Roll));
			CurSpeed += ToStopAccelerate * DeltaTime;
			if (CurSpeed <= 0.f)
			{
				CurSpeed = 0.f;
				IsOver = false;
				ToStopAccelerate = -AccelerateSpeed * 2;  //恢复默认的返回加速度
			}
		}
	}

	if (CurSpeed == 0.f)
	{
		StopLengthTime += DeltaTime;
		if (StopLengthTime >= 5.f)
		{
			ForceActive = true;
			StopLengthTime = 0.f;
		}
	}
	else
		ForceActive = false;
}

void AFlyObstacle::Destroyed()
{
	GetWorldTimerManager().ClearTimer(QueryAngler);
	Super::Destroyed();
}

void AFlyObstacle::QueryIsOverSubAngle()
{
	if ((AimCharacter != nullptr && !IsOver) || ForceActive)  //只有在非超过时才执行下面的操作
	{
		bool IsOnPhysicPlatform = false;
		if (AimCharacter->Controller)
		{
			AMyPlayerController* MPC = Cast<AMyPlayerController>(AimCharacter->Controller);
			IsOnPhysicPlatform = MPC->CurPlatform->IsA(MPC->SpawnPlatform_Physic);    //玩家当前平台是物理平台就停止移动
		}

		if (IsOnPhysicPlatform == false)
		{
			FVector CurBestDir = (AimCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
			float SubAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CurBestDir, FlyDir)));    //算出相差角度
																											   //GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::FormatAsNumber(SubAngle));

			if ((SubAngle > 30.f && SubAngle < 150.f) || ForceActive)
			{
				FlyDir = CurBestDir;   //更改飞行角度
				FlyDst = AimCharacter->GetActorLocation();    //更改飞行的目标位置
			}
		}
	}
	GetWorldTimerManager().SetTimer(QueryAngler, this, &AFlyObstacle::QueryIsOverSubAngle, 0.5f, false);    //每0.5秒检查一次
}

void AFlyObstacle::StartDestroy()
{
	ShouldShowDir = false;
	if (!FlyObstacleMesh->IsSimulatingPhysics())
		FlyObstacleMesh->SetSimulatePhysics(true);

	ARunGameState* RGS = Cast<ARunGameState>(GetWorld()->GetGameState());
	if (RGS)
		RGS->UpdatePlayerScore(1000.f);     //摧毁一个飞行障碍就加1000分

	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AFlyObstacle::DestroyActor, 4.f, false);
}

void AFlyObstacle::DestroyActor()
{
	GetWorldTimerManager().ClearTimer(QueryAngler);
	GetWorldTimerManager().ClearTimer(ShowDir);

	Destroy();
}

void AFlyObstacle::SelectSuitStopAccelerate(FVector MoveDir, float CurSpeed, float MoveDistance)
{
	FHitResult Result;
	FCollisionObjectQueryParams ObjectQueryParams(ECollisionChannel::ECC_WorldDynamic);       //只检测平台网格,平台网格是WorldStatic
	FCollisionQueryParams QueryParams(TEXT("ObstacleQuery"));
	GetWorld()->SweepSingleByObjectType(Result,
										GetActorLocation() + MoveDir * MoveDistance + FVector(0.f, 0.f, 1.f)*400.f,
										GetActorLocation() + MoveDir * MoveDistance + FVector(0.f, 0.f, -1.f)*400.f,
										FQuat::Identity, ObjectQueryParams, 
										FCollisionShape::MakeBox(HalfObstacleSize), 
										QueryParams);

	if (Cast<ARunPlatform>(Result.GetActor()))      //检测到平台
	{
		//先加移动距离200
		MoveDistance += 200.f;
		SelectSuitStopAccelerate(MoveDir, CurSpeed, MoveDistance);
		GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Black, TEXT("调整加速度"));
	} 
	else
	{
		ToStopAccelerate = ComputeAccelerateToStop(CurSpeed, MoveDistance);
	}
}

float AFlyObstacle::ComputeDistanceToStop(float CurSpeed, float Accelerate)
{
	float MoveTime = FMath::Abs(CurSpeed / Accelerate);
	return FMath::Abs(Accelerate * MoveTime * MoveTime) / 2;       //返回移动距离
}

float AFlyObstacle::ComputeAccelerateToStop(float CurSpeed, float MoveDistance)
{
	return -(CurSpeed * CurSpeed) / (MoveDistance * 2);       //求出合适的加速度
}

void AFlyObstacle::ShowObstaclePosRelativeToChar()
{
	if (AimCharacter != nullptr && ShouldShowDir)
	{
		const FMatrix CharOrient = FRotationMatrix(AimCharacter->GetActorRotation());
		
		const FVector CharFront = CharOrient.GetUnitAxis(EAxis::X);    //玩家正前方向
		const FVector CharBehind = -CharFront;     //正后方
		const FVector CharRight = CharOrient.GetUnitAxis(EAxis::Y);    //玩家正右方
		const FVector CharLeft = -CharRight;  //玩家正左边

		const FVector CharToObtsacleDir = (GetActorLocation() - AimCharacter->GetActorLocation()).GetSafeNormal2D();   //障碍物相对于玩家的方向

		float FrontDegrees = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CharFront, CharToObtsacleDir)));
		if (FrontDegrees < 45.f)
			CurDirToChar = EFlyObstacleToCharDir::Type::FOTCD_Front;

		float RightDegrees = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CharRight, CharToObtsacleDir)));
		if (RightDegrees < 45.f)
			CurDirToChar = EFlyObstacleToCharDir::Type::FOTCD_Right;

		float BehindDegrees = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CharBehind, CharToObtsacleDir)));
		if (BehindDegrees < 45.f)
			CurDirToChar = EFlyObstacleToCharDir::Type::FOTCD_Behind;

		float LeftDegrees = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CharLeft, CharToObtsacleDir)));
		if (LeftDegrees < 45.f)
			CurDirToChar = EFlyObstacleToCharDir::Type::FOTCD_Left;

		switch(CurDirToChar)
		{
		case EFlyObstacleToCharDir::FOTCD_Front:
			GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("在前方! 速度为%4.2f m/s"), CurSpeed / 100.f));
			break;

		case EFlyObstacleToCharDir::FOTCD_Right:
			GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("在右方! 速度为%4.2f m/s"), CurSpeed / 100.f));
			break;

		case EFlyObstacleToCharDir::FOTCD_Behind:
			GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("在后方! 速度为%4.2f m/s"), CurSpeed / 100.f));
			break;

		case EFlyObstacleToCharDir::FOTCD_Left:
			GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("在左方! 速度为%4.2f m/s"), CurSpeed / 100.f));
			break;
		}

		GetWorldTimerManager().SetTimer(ShowDir, this, &AFlyObstacle::ShowObstaclePosRelativeToChar, 1.f, true);
	}
}
