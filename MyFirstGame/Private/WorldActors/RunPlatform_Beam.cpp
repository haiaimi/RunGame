// Fill out your copyright notice in the Description page of Project Settings.

#include "RunPlatform_Beam.h"
#include "Particles/ParticleSystem.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Classes/Particles/ParticleSystemComponent.h"
#include "MyFirstGame.h"
#include "Bullet.h"
#include "Engine/Engine.h"
#include "Weapon_Gun.h"
#include "ConstructorHelpers.h"
#include "Curves/CurveFloat.h"
#include "MyFirstGameCharacter.h"
#include "Player/MyPlayerController.h"

ARunPlatform_Beam::ARunPlatform_Beam(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	AttachmentParticle = ObjectInitializer.CreateDefaultSubobject<UParticleSystem>(this, TEXT("AttachmentParticle"));
	AttachmentTrigger = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("AttachmentTrigger"));
	AttachmentTrigger->SetupAttachment(Platform);

	//设置检测体的碰撞响应
	AttachmentTrigger->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);    //仅用于检测
	AttachmentTrigger->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AttachmentTrigger->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Overlap);      //仅检测子弹

	UpdateBeam = false;
	IsInMove = false;

	static ConstructorHelpers::FObjectFinder<UCurveFloat> FindMoveCurve(TEXT("/Game/Blueprint/MoveCurve"));
	MoveCurve = FindMoveCurve.Object;

	SlopeAngle = 0.f;   //该平台不进行旋转
	NoPlayerToSlope = true;  //该平台不需要旋转
	IsMoveUp = false;  //默认方向是左右移动

}

void ARunPlatform_Beam::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ToParticle = UGameplayStatics::SpawnEmitterAttached(AttachmentParticle, Platform, AttachSocket);
	ToParticle->SetWorldScale3D(FVector(1.f, 1.f, 1.f));
	ToParticle->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));

	float Width = GetPlatformWidth();
	AttachmentTrigger->SetWorldScale3D(FVector(2.5f, 2.5f, 2.5f));
	AttachmentTrigger->SetRelativeLocation(FVector(GetPlatformLength() / XScale, Width / 2, 0.f));
	AttachmentTrigger->OnComponentBeginOverlap.AddDynamic(this, &ARunPlatform_Beam::AttachBeginOverlap);    //添加碰撞

}

void ARunPlatform_Beam::BeginPlay()
{
	Super::BeginPlay();

	IsInMove = true;
	TempSpawnLocation = SpawnLocation;

	if (IsMoveUp)
		MoveDir = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Z);
	else
		MoveDir = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Y);
}

void ARunPlatform_Beam::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	
	if (UpdateBeam && TargetGun != nullptr && BeamParticle)
	{
		const FVector BeamSourcePoint = Platform->GetSocketLocation(AttachSocket);
		const FVector BeamTargetPoint = TargetGun->GetFireLocation();
		BeamParticle->SetBeamTargetPoint(0, BeamTargetPoint, 0);    //更新闪电目标

		if ((BeamSourcePoint - BeamTargetPoint).Size() >= 2000.f)
			DeActiveBeam();
	}
	if (MoveCurve && IsInMove && !MoveToOrigin)
	{
		float RelativeDistance = MoveCurve->GetFloatValue(MoveCycle) * 100;
		MoveCycle += DeltaTime;
		SetActorLocation(SpawnLocation + MoveDir * RelativeDistance);

		if (MoveCycle >= 6)
			MoveCycle = 0.f;
	}
}

void ARunPlatform_Beam::AttachBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	ABullet* Temp = Cast<ABullet>(OtherActor);

	if (FVector::DotProduct(SweepResult.ImpactNormal, FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::X)) < 0)     //判断子弹是否从粒子背面射过来
		if (Temp != nullptr)
		{
			if (Temp->CurWeaponType == EWeaponType::Weapon_Beam)
			{
				BeamParticle = UGameplayStatics::SpawnEmitterAttached(Temp->OwnerWeapon->BeamEmitter, Platform, AttachSocket);    //再生成一个闪电粒子
				BeamParticle->SetWorldScale3D(FVector(5.f, 5.f, 5.f));
				BeamParticle->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
				TargetGun = Temp->OwnerWeapon;
				UpdateBeam = true;
				IsInMove = false;

				MoveToPlayerPlat();
				StopLocation = GetActorLocation();
				Temp->Destroy();
			}
		}
}

void ARunPlatform_Beam::MoveTick(float DeltaTime)
{
	if (MoveToNew)
	{
		if (IsInMove)
		{
			FVector DstLoc = TempSpawnLocation + DeltaLoc;
			
			SpawnLocation = FMath::VInterpTo(SpawnLocation, DstLoc, DeltaTime, 10.f);

				if (NextPlatform != nullptr)
					if (!NextPlatform->MoveToNew)     //只有下一个平台没有移动时才执行下面操作
						if ((SpawnLocation - TempSpawnLocation).Size() > DeltaLoc.Size() / 2)  //移动超过相差距离一半时，就开始移动下一个平台
							NextPlatform->MoveToNewPos(DeltaLoc);

			if ((SpawnLocation - DstLoc).Size() < 1.f) //平台到达目标位置时    注意浮点数、注意浮点数、注意浮点数！！！重要的事说三遍
			{
				MoveToNew = false;  //停止移动更新
				TempSpawnLocation = SpawnLocation;

				AMyPlayerController* MPC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
				if (MPC != nullptr)
				{
					int32 StopIndex = MPC->PlatformArray.Find(this);

					for (int32 i = 0; i < StopIndex; i++)
					{
						if (MPC->PlatformArray[i] != nullptr)
							MPC->PlatformArray[i]->MoveToNew = false;
					}
				}
			}
		}
		else
		{
			FVector NewPos = FMath::VInterpTo(GetActorLocation(), StopLocation + DeltaLoc, DeltaTime, 10.f);
			SetActorLocation(NewPos);

			if (NextPlatform != nullptr)
				if (!NextPlatform->MoveToNew)   //只有下一个平台没有移动时才执行下面操作
					if ((NewPos - StopLocation).Size() > DeltaLoc.Size() / 2)   //移动超过相差距离一半时，就开始移动下一个平台
						NextPlatform->MoveToNewPos(NextPlatToCur);

			if ((NewPos - (StopLocation + DeltaLoc)).Size() < 1.f)
				MoveToNew = false;  //停止更新
		}
	}
}

void ARunPlatform_Beam::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	Super::BeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	//玩家进入该平台时，删掉闪电粒子
	if (CurChar != nullptr && BeamParticle != nullptr)
	{
		BeamParticle->SetVisibility(false);
		BeamParticle->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}

	ARunPlatform* TouchedPlatform = Cast<ARunPlatform>(OtherActor);
	if (TouchedPlatform)
	{
		FVector CurDirY = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Y);    //获得Y方向的向量
		FVector TouchDirY = FRotationMatrix(TouchedPlatform->GetActorRotation()).GetUnitAxis(EAxis::Y);   //获取碰撞的平台的Y方向
		//下面是获取两个平台位置的X分量
		float CurLocationX = SpawnLocation.X;
		float TouchLocationX = TouchedPlatform->SpawnLocation.X;

		if ((CurDirY + TouchDirY).IsNearlyZero() && !TouchedPlatform->MoveToNew)
		{
			if (this->PlatDir == EPlatformDirection::Absolute_Left && (CurLocationX - TouchLocationX) > 0)    //这是当前平台向左（绝对方向），并且被碰撞的平台在其右边（相对方向）
				SpawnLocation += -50 * CurDirY;

			if (this->PlatDir == EPlatformDirection::Absolute_Right && (CurLocationX - TouchLocationX) > 0)    //这是当前平台向右（绝对方向），并且被碰撞的平台在其右边（相对方向)
				SpawnLocation += 50 * CurDirY;

			if (this->PlatDir == EPlatformDirection::Absolute_Right && (CurLocationX - TouchLocationX) < 0)    //这是当前平台向右（绝对方向），并且被碰撞的平台在其左边（相对方向）
				SpawnLocation += -50 * CurDirY;

			if (this->PlatDir == EPlatformDirection::Absolute_Left && (CurLocationX - TouchLocationX) < 0)    //这是当前平台向左（绝对方向），并且被碰撞的平台在其左边（相对方向）
				SpawnLocation += 50 * CurDirY;
		}
	}

}

void ARunPlatform_Beam::StartDestroy()
{
	Super::StartDestroy();

	AMyPlayerController* MC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());

	if (MC != nullptr)
	{
		MC->InConnectedToPlat = false;
		MC->CurConnectedPlat = nullptr;
	}
}

void ARunPlatform_Beam::MoveToPlayerPlat()
{
	AMyPlayerController* MC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());

	if (MC != nullptr)
	{
		MC->InConnectedToPlat = true;
		MC->CurConnectedPlat = this;
		//GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Black, FString::Printf(TEXT("Platform Num: %d"), MC->PlatformArray.Num()));
		if (MC->CurPlatform != nullptr)
		{
			FVector ToPlayerPos = MC->CurPlatform->GetActorLocation() - GetActorLocation();      //需要移动的距离

			FVector DeltaPos;
			int32 PlatformLength = GetPlatformLength();

			if (MC->CurPlatform->PlatDir == EPlatformDirection::Absolute_Left)
			{
				DeltaPos = FVector(0.f, ToPlayerPos.Y - 2 * PlatformLength, ToPlayerPos.Z);
				MoveToNewPos(FVector(0.f, ToPlayerPos.Y - PlatformLength, ToPlayerPos.Z));
			}
				
			else if (MC->CurPlatform->PlatDir == EPlatformDirection::Absolute_Forward)
			{
				DeltaPos = FVector(ToPlayerPos.X + 2 * PlatformLength, 0, ToPlayerPos.Z);
				MoveToNewPos(FVector(ToPlayerPos.X + PlatformLength, 0, ToPlayerPos.Z));
			}

			else if (MC->CurPlatform->PlatDir == EPlatformDirection::Absolute_Right)
			{
				DeltaPos = FVector(0.f, ToPlayerPos.Y + 2 * PlatformLength, ToPlayerPos.Z);
				MoveToNewPos(FVector(0.f, ToPlayerPos.Y + PlatformLength, ToPlayerPos.Z));
			}

			if (NextPlatform != nullptr)
				NextPlatToCur = GetActorLocation() + DeltaPos - NextPlatform->GetActorLocation();    //求出下一个普通平台到该平台的相对位置
		}
	}
}

void ARunPlatform_Beam::DeActiveBeam()
{
	if (BeamParticle != nullptr)
		BeamParticle->SetVisibility(false);
}

void ARunPlatform_Beam::MoveToAllFun(const FVector DeltaDistance)
{
	Super::MoveToAllFun(DeltaDistance);

	IsInMove = false;
	SpawnLocation = GetActorLocation();
	if (ToParticle != nullptr)
		ToParticle->SetVisibility(false);
}

void ARunPlatform_Beam::StopToAllFun(const FVector DeltaDistance)
{
	Super::StopToAllFun(DeltaDistance);

	NoPlayerToSlope = true;
	IsInMove = true;
	MoveCycle = 0.f;
	if (ToParticle != nullptr)
		ToParticle->SetVisibility(true);
}

void ARunPlatform_Beam::MoveToAllTick(float DeltaTime)
{
	Super::MoveToAllTick(DeltaTime);

	TempSpawnLocation = SpawnLocation;
}

void ARunPlatform_Beam::MoveToOriginTick(float DeltaTime)
{
	Super::MoveToOriginTick(DeltaTime);

	TempSpawnLocation = SpawnLocation;
}