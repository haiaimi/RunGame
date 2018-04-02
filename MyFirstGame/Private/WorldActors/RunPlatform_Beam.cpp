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

	//���ü�������ײ��Ӧ
	AttachmentTrigger->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);    //�����ڼ��
	AttachmentTrigger->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AttachmentTrigger->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Overlap);      //������ӵ�

	UpdateBeam = false;
	IsInMove = false;

	static ConstructorHelpers::FObjectFinder<UCurveFloat> FindMoveCurve(TEXT("/Game/Blueprint/MoveCurve"));
	MoveCurve = FindMoveCurve.Object;

	SlopeAngle = 0.f;   //��ƽ̨��������ת
	NoPlayerToSlope = true;  //��ƽ̨����Ҫ��ת
	IsMoveUp = false;  //Ĭ�Ϸ����������ƶ�

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
	AttachmentTrigger->OnComponentBeginOverlap.AddDynamic(this, &ARunPlatform_Beam::AttachBeginOverlap);    //�����ײ

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
		BeamParticle->SetBeamTargetPoint(0, BeamTargetPoint, 0);    //��������Ŀ��

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

	if (FVector::DotProduct(SweepResult.ImpactNormal, FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::X)) < 0)     //�ж��ӵ��Ƿ�����ӱ��������
		if (Temp != nullptr)
		{
			if (Temp->CurWeaponType == EWeaponType::Weapon_Beam)
			{
				BeamParticle = UGameplayStatics::SpawnEmitterAttached(Temp->OwnerWeapon->BeamEmitter, Platform, AttachSocket);    //������һ����������
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
					if (!NextPlatform->MoveToNew)     //ֻ����һ��ƽ̨û���ƶ�ʱ��ִ���������
						if ((SpawnLocation - TempSpawnLocation).Size() > DeltaLoc.Size() / 2)  //�ƶ�����������һ��ʱ���Ϳ�ʼ�ƶ���һ��ƽ̨
							NextPlatform->MoveToNewPos(DeltaLoc);

			if ((SpawnLocation - DstLoc).Size() < 1.f) //ƽ̨����Ŀ��λ��ʱ    ע�⸡������ע�⸡������ע�⸡������������Ҫ����˵����
			{
				MoveToNew = false;  //ֹͣ�ƶ�����
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
				if (!NextPlatform->MoveToNew)   //ֻ����һ��ƽ̨û���ƶ�ʱ��ִ���������
					if ((NewPos - StopLocation).Size() > DeltaLoc.Size() / 2)   //�ƶ�����������һ��ʱ���Ϳ�ʼ�ƶ���һ��ƽ̨
						NextPlatform->MoveToNewPos(NextPlatToCur);

			if ((NewPos - (StopLocation + DeltaLoc)).Size() < 1.f)
				MoveToNew = false;  //ֹͣ����
		}
	}
}

void ARunPlatform_Beam::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	Super::BeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	//��ҽ����ƽ̨ʱ��ɾ����������
	if (CurChar != nullptr && BeamParticle != nullptr)
	{
		BeamParticle->SetVisibility(false);
		BeamParticle->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}

	ARunPlatform* TouchedPlatform = Cast<ARunPlatform>(OtherActor);
	if (TouchedPlatform)
	{
		FVector CurDirY = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Y);    //���Y���������
		FVector TouchDirY = FRotationMatrix(TouchedPlatform->GetActorRotation()).GetUnitAxis(EAxis::Y);   //��ȡ��ײ��ƽ̨��Y����
		//�����ǻ�ȡ����ƽ̨λ�õ�X����
		float CurLocationX = SpawnLocation.X;
		float TouchLocationX = TouchedPlatform->SpawnLocation.X;

		if ((CurDirY + TouchDirY).IsNearlyZero() && !TouchedPlatform->MoveToNew)
		{
			if (this->PlatDir == EPlatformDirection::Absolute_Left && (CurLocationX - TouchLocationX) > 0)    //���ǵ�ǰƽ̨���󣨾��Է��򣩣����ұ���ײ��ƽ̨�����ұߣ���Է���
				SpawnLocation += -50 * CurDirY;

			if (this->PlatDir == EPlatformDirection::Absolute_Right && (CurLocationX - TouchLocationX) > 0)    //���ǵ�ǰƽ̨���ң����Է��򣩣����ұ���ײ��ƽ̨�����ұߣ���Է���)
				SpawnLocation += 50 * CurDirY;

			if (this->PlatDir == EPlatformDirection::Absolute_Right && (CurLocationX - TouchLocationX) < 0)    //���ǵ�ǰƽ̨���ң����Է��򣩣����ұ���ײ��ƽ̨������ߣ���Է���
				SpawnLocation += -50 * CurDirY;

			if (this->PlatDir == EPlatformDirection::Absolute_Left && (CurLocationX - TouchLocationX) < 0)    //���ǵ�ǰƽ̨���󣨾��Է��򣩣����ұ���ײ��ƽ̨������ߣ���Է���
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
			FVector ToPlayerPos = MC->CurPlatform->GetActorLocation() - GetActorLocation();      //��Ҫ�ƶ��ľ���

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
				NextPlatToCur = GetActorLocation() + DeltaPos - NextPlatform->GetActorLocation();    //�����һ����ͨƽ̨����ƽ̨�����λ��
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