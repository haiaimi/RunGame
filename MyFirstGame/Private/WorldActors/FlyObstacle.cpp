// Fill out your copyright notice in the Description page of Project Settings.

#include "FlyObstacle.h"
#include "Components/StaticMeshComponent.h"
#include "MyFirstGameCharacter.h"
#include "TimerManager.h"
#include "Engine/Engine.h"


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

	FVector ObstacleSize = FlyObstacleMesh->Bounds.BoxExtent;
	FlyObstacleMesh->SetRelativeLocation(FVector(-ObstacleSize.X, -ObstacleSize.Y, 0.f));
}

// Called when the game starts or when spawned
void AFlyObstacle::BeginPlay()
{
	Super::BeginPlay();
	
	
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
	}
	else      //这是超过目标点的情况
	{
		if (CurSpeed > 0)
		{
			IsOver = true;
			SetActorLocation(GetActorLocation() + FlyDir * CurSpeed * DeltaTime);

			const FRotator ObstacleRotation = GetActorRotation();
			SetActorRotation(FRotator(ObstacleRotation.Pitch, ObstacleRotation.Yaw + CurSpeed * DeltaTime, ObstacleRotation.Roll));
			CurSpeed -= 2 * AccelerateSpeed * DeltaTime;
			if (CurSpeed <= 0)
			{
				CurSpeed = 0;
				IsOver = false;
			}
		}
	}
}

void AFlyObstacle::Destroyed()
{
	GetWorldTimerManager().ClearTimer(QueryAngler);
	Super::Destroyed();
}

void AFlyObstacle::QueryIsOverSubAngle()
{
	if (AimCharacter != nullptr && !IsOver)  //只有在非超过时才执行下面的操作
	{
		FVector CurBestDir = (AimCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		float SubAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CurBestDir, FlyDir)));    //算出相差角度
		GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::FormatAsNumber(SubAngle));

		if (SubAngle > 30.f && SubAngle < 150.f)
		{
			FlyDir = CurBestDir;   //更改飞行角度
			FlyDst = AimCharacter->GetActorLocation();    //更改飞行的目标位置
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, TEXT("Query Start"));
	GetWorldTimerManager().SetTimer(QueryAngler, this, &AFlyObstacle::QueryIsOverSubAngle, 0.5f, false);    //每0.5秒检查一次
}

void AFlyObstacle::StartDestroy()
{
	if (!FlyObstacleMesh->IsSimulatingPhysics())
		FlyObstacleMesh->SetSimulatePhysics(true);

	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AFlyObstacle::DestroyActor, 4.f, false);
}

void AFlyObstacle::DestroyActor()
{
	GetWorldTimerManager().ClearTimer(QueryAngler);
	Destroy();
}