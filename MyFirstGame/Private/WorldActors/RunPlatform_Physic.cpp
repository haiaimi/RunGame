// Fill out your copyright notice in the Description page of Project Settings.

#include "RunPlatform_Physic.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "MyFirstGameCharacter.h"

ARunPlatform_Physic::ARunPlatform_Physic(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	AttachedMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("AttachedMesh"));
	ConstraintComponent = ObjectInitializer.CreateDefaultSubobject<UPhysicsConstraintComponent>(this, TEXT("ConstraintComponent"));
	LinkParticle = ObjectInitializer.CreateDefaultSubobject<UParticleSystem>(this, TEXT("LinkParticle"));

	AttachedMesh->SetupAttachment(ArrowDst);  //�����ڼ�ͷ���

	//��������Լ�����������
	ConstraintComponent->ComponentName1.ComponentName = TEXT("AttachedMesh");
	ConstraintComponent->ComponentName2.ComponentName = TEXT("Platform");
	ConstraintComponent->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, 45.f);
	ConstraintComponent->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, 45.f);
	ConstraintComponent->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Limited, 45.f);
	ConstraintComponent->SetupAttachment(ArrowDst);
	
	XScale = 1;
	YScale = 1;

	NoPlayerToSlope = true;
	SlopeAngle = 0.f;
}

void ARunPlatform_Physic::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (LinkParticle != NULL)
		SpawnedParticle = UGameplayStatics::SpawnEmitterAttached(LinkParticle, Platform, LinkSocket);

}

void ARunPlatform_Physic::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (LinkParticle != NULL && SpawnedParticle != NULL)
		SpawnedParticle->SetBeamTargetPoint(0, AttachedMesh->GetComponentLocation() - FVector(0.f, 0.f, 50.f), 0);
}

void ARunPlatform_Physic::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	Super::BeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (CurChar != NULL)
	{
		Platform->SetSimulatePhysics(true);
	}
}

void ARunPlatform_Physic::EndOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	Super::EndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	if (Cast<AMyFirstGameCharacter>(OtherActor) != NULL)
	{
		SpawnedParticle->SetVisibility(false);
		AttachedMesh->SetSimulatePhysics(true);
		ConstraintComponent->BreakConstraint();    //ȡ��Լ��
	}
}

void ARunPlatform_Physic::MoveTick(float DeltaTime)
{
	Super::MoveTick(DeltaTime);
}

