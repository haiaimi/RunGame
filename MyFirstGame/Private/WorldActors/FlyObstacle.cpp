// Fill out your copyright notice in the Description page of Project Settings.

#include "FlyObstacle.h"
#include "Components/StaticMeshComponent.h"
#include "MyFirstGameCharacter.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "RunPlatform.h"


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
	FlyObstacleMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);         //ֻ����Ҳ�����ײ

	//Ĭ�ϳ��ٶ�����ٶ�
	StartSpeed = CurSpeed = 0.f;
	AccelerateSpeed = 200.f;
	ToStopAccelerate = -AccelerateSpeed * 2;
	IsOver = false;
}

void AFlyObstacle::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (AimCharacter != nullptr)
	{
		FlyDir = (AimCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();         //���ó�ʼ�ķ���
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
	
//������������
#ifdef WITH_EDITOR
	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Black, TEXT("In Editor"));
#elif PLATFORM_DESKTOP
	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Black, TEXT("In Windows"));

#endif // WITH_EDITOR

	//FCollisionObjectQueryParams
	//FCollisionQueryParams
	//GetWorld()->Sweep
}

// Called every frame
void AFlyObstacle::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (FVector::DotProduct(FlyDir, (GetActorLocation() - FlyDst).GetSafeNormal2D()) < 0)        //���ǻ�δ����Ŀ�������
	{
		SetActorLocation(GetActorLocation() + FlyDir * CurSpeed * DeltaTime);

		const FRotator ObstacleRotation = GetActorRotation();
		SetActorRotation(FRotator(ObstacleRotation.Pitch, ObstacleRotation.Yaw + CurSpeed * DeltaTime/2.f , ObstacleRotation.Roll));  //ͬʱ�����ƶ��ٶ���ת
		CurSpeed += AccelerateSpeed * DeltaTime;
		if (CurSpeed >= 2000.f)
			CurSpeed = 2000.f;
	}
	else          //���ǳ���Ŀ�������
	{
		if (CurSpeed > 0)
		{
			IsOver = true;
			SetActorLocation(GetActorLocation() + FlyDir * CurSpeed * DeltaTime);

			const FRotator ObstacleRotation = GetActorRotation();
			SetActorRotation(FRotator(ObstacleRotation.Pitch, ObstacleRotation.Yaw + CurSpeed * DeltaTime, ObstacleRotation.Roll));
			CurSpeed += ToStopAccelerate;
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
	if (AimCharacter != nullptr && !IsOver)  //ֻ���ڷǳ���ʱ��ִ������Ĳ���
	{
		FVector CurBestDir = (AimCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		float SubAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CurBestDir, FlyDir)));    //������Ƕ�
		GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::FormatAsNumber(SubAngle));

		if (SubAngle > 30.f && SubAngle < 150.f)
		{
			FlyDir = CurBestDir;   //���ķ��нǶ�
			FlyDst = AimCharacter->GetActorLocation();    //���ķ��е�Ŀ��λ��
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, TEXT("Query Start"));
	GetWorldTimerManager().SetTimer(QueryAngler, this, &AFlyObstacle::QueryIsOverSubAngle, 0.5f, false);    //ÿ0.5����һ��
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

void AFlyObstacle::SelectSuitStopLocation(FVector MoveDir, float MoveDistance)
{
	FHitResult Result;
	FCollisionObjectQueryParams ObjectQueryParams(ECollisionChannel::ECC_WorldDynamic);       //ֻ���ƽ̨����,ƽ̨������WorldStatic
	FCollisionQueryParams QueryParams(TEXT("ObstacleQuery"));
	GetWorld()->SweepSingleByObjectType(Result,
										GetActorLocation() + MoveDir * MoveDistance,
										GetActorLocation() + MoveDir * MoveDistance + FVector(0.f, 0.f, -1.f)*200.f,
										FQuat::Identity, ObjectQueryParams, 
										FCollisionShape::MakeBox(HalfObstacleSize), 
										QueryParams);

	if (Cast<ARunPlatform>(Result.GetActor()))      //��⵽ƽ̨
	{
		//�ȼ��ƶ�����200
	}
}

float AFlyObstacle::ComputeDistanceToStop(float CurSpeed, float Accelerate)
{
	float MoveTime = FMath::Abs(CurSpeed / Accelerate);
	return FMath::Abs(Accelerate*MoveTime*MoveTime) / 2;       //�����ƶ�����
}

float AFlyObstacle::ComputeAccelerateToStop(float CurSpeed, float MoveDistance)
{
	return -CurSpeed * CurSpeed / MoveDistance / 2;       //������ʵļ��ٶ�
}
