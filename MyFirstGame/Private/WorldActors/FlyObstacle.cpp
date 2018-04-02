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
	FlyObstacleMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);         //ֻ����Ҳ�����ײ

	//Ĭ�ϳ��ٶ�����ٶ�
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

	ShowObstaclePosRelativeToChar();
	ToStopAccelerate = -AccelerateSpeed * 2;
//������������
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

	if (FVector::DotProduct(FlyDir, (GetActorLocation() - FlyDst).GetSafeNormal2D()) < 0)        //���ǻ�δ����Ŀ�������
	{
		SetActorLocation(GetActorLocation() + FlyDir * CurSpeed * DeltaTime);

		const FRotator ObstacleRotation = GetActorRotation();
		SetActorRotation(FRotator(ObstacleRotation.Pitch, ObstacleRotation.Yaw + CurSpeed * DeltaTime/2.f , ObstacleRotation.Roll));  //ͬʱ�����ƶ��ٶ���ת
		CurSpeed += AccelerateSpeed * DeltaTime;
		if (CurSpeed >= 2000.f)
			CurSpeed = 2000.f;

		ToCharMaxSpeed = CurSpeed;
	}
	else          //���ǳ���Ŀ�������
	{
		if (CurSpeed > 0)
		{
			if (ToCharMaxSpeed == CurSpeed)
			{
				if (ToCharMaxSpeed < 1000.f)
				{
					CurSpeed = ToCharMaxSpeed = 1000.f;      //ʹ����С�ļ��ٳ��ٶ�Ϊ500����ֹͣ��ƽ̨�Ϲ���ʱ��
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
				ToStopAccelerate = -AccelerateSpeed * 2;  //�ָ�Ĭ�ϵķ��ؼ��ٶ�
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
	if ((AimCharacter != nullptr && !IsOver) || ForceActive)  //ֻ���ڷǳ���ʱ��ִ������Ĳ���
	{
		bool IsOnPhysicPlatform = false;
		if (AimCharacter->Controller)
		{
			AMyPlayerController* MPC = Cast<AMyPlayerController>(AimCharacter->Controller);
			IsOnPhysicPlatform = MPC->CurPlatform->IsA(MPC->SpawnPlatform_Physic);    //��ҵ�ǰƽ̨������ƽ̨��ֹͣ�ƶ�
		}

		if (IsOnPhysicPlatform == false)
		{
			FVector CurBestDir = (AimCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
			float SubAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CurBestDir, FlyDir)));    //������Ƕ�
																											   //GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::FormatAsNumber(SubAngle));

			if ((SubAngle > 30.f && SubAngle < 150.f) || ForceActive)
			{
				FlyDir = CurBestDir;   //���ķ��нǶ�
				FlyDst = AimCharacter->GetActorLocation();    //���ķ��е�Ŀ��λ��
			}
		}
	}
	GetWorldTimerManager().SetTimer(QueryAngler, this, &AFlyObstacle::QueryIsOverSubAngle, 0.5f, false);    //ÿ0.5����һ��
}

void AFlyObstacle::StartDestroy()
{
	ShouldShowDir = false;
	if (!FlyObstacleMesh->IsSimulatingPhysics())
		FlyObstacleMesh->SetSimulatePhysics(true);

	ARunGameState* RGS = Cast<ARunGameState>(GetWorld()->GetGameState());
	if (RGS)
		RGS->UpdatePlayerScore(1000.f);     //�ݻ�һ�������ϰ��ͼ�1000��

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
	FCollisionObjectQueryParams ObjectQueryParams(ECollisionChannel::ECC_WorldDynamic);       //ֻ���ƽ̨����,ƽ̨������WorldStatic
	FCollisionQueryParams QueryParams(TEXT("ObstacleQuery"));
	GetWorld()->SweepSingleByObjectType(Result,
										GetActorLocation() + MoveDir * MoveDistance + FVector(0.f, 0.f, 1.f)*400.f,
										GetActorLocation() + MoveDir * MoveDistance + FVector(0.f, 0.f, -1.f)*400.f,
										FQuat::Identity, ObjectQueryParams, 
										FCollisionShape::MakeBox(HalfObstacleSize), 
										QueryParams);

	if (Cast<ARunPlatform>(Result.GetActor()))      //��⵽ƽ̨
	{
		//�ȼ��ƶ�����200
		MoveDistance += 200.f;
		SelectSuitStopAccelerate(MoveDir, CurSpeed, MoveDistance);
		GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Black, TEXT("�������ٶ�"));
	} 
	else
	{
		ToStopAccelerate = ComputeAccelerateToStop(CurSpeed, MoveDistance);
	}
}

float AFlyObstacle::ComputeDistanceToStop(float CurSpeed, float Accelerate)
{
	float MoveTime = FMath::Abs(CurSpeed / Accelerate);
	return FMath::Abs(Accelerate * MoveTime * MoveTime) / 2;       //�����ƶ�����
}

float AFlyObstacle::ComputeAccelerateToStop(float CurSpeed, float MoveDistance)
{
	return -(CurSpeed * CurSpeed) / (MoveDistance * 2);       //������ʵļ��ٶ�
}

void AFlyObstacle::ShowObstaclePosRelativeToChar()
{
	if (AimCharacter != nullptr && ShouldShowDir)
	{
		const FMatrix CharOrient = FRotationMatrix(AimCharacter->GetActorRotation());
		
		const FVector CharFront = CharOrient.GetUnitAxis(EAxis::X);    //�����ǰ����
		const FVector CharBehind = -CharFront;     //����
		const FVector CharRight = CharOrient.GetUnitAxis(EAxis::Y);    //������ҷ�
		const FVector CharLeft = -CharRight;  //��������

		const FVector CharToObtsacleDir = (GetActorLocation() - AimCharacter->GetActorLocation()).GetSafeNormal2D();   //�ϰ����������ҵķ���

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
			GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("��ǰ��! �ٶ�Ϊ%4.2f m/s"), CurSpeed / 100.f));
			break;

		case EFlyObstacleToCharDir::FOTCD_Right:
			GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("���ҷ�! �ٶ�Ϊ%4.2f m/s"), CurSpeed / 100.f));
			break;

		case EFlyObstacleToCharDir::FOTCD_Behind:
			GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("�ں�! �ٶ�Ϊ%4.2f m/s"), CurSpeed / 100.f));
			break;

		case EFlyObstacleToCharDir::FOTCD_Left:
			GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("����! �ٶ�Ϊ%4.2f m/s"), CurSpeed / 100.f));
			break;
		}

		GetWorldTimerManager().SetTimer(ShowDir, this, &AFlyObstacle::ShowObstaclePosRelativeToChar, 1.f, true);
	}
}
