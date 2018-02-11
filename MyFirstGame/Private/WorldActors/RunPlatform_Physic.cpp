// Fill out your copyright notice in the Description page of Project Settings.

#include "RunPlatform_Physic.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

ARunPlatform_Physic::ARunPlatform_Physic(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	AttachedMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("AttachedMesh"));
	ConstraintComponent = ObjectInitializer.CreateDefaultSubobject<UPhysicsConstraintComponent>(this, TEXT("ConstraintComponent"));
	LinkParticle = ObjectInitializer.CreateDefaultSubobject<UParticleSystem>(this, TEXT("LinkParticle"));

	AttachedMesh->SetupAttachment(ArrowDst);  //依附于箭头组件

	//下面设置约束组件的属性
	FConstrainComponentPropName ConstraintName1, ConstraintName2;
	ConstraintName1.ComponentName = TEXT("AttachedMesh");
	ConstraintName2.ComponentName = TEXT("Platform");
	ConstraintComponent->ComponentName1 = ConstraintName1;
	ConstraintComponent->ComponentName2 = ConstraintName2;
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

	Platform->SetSimulatePhysics(true);       //需要开启平台网格的物理模拟
	if (LinkParticle != NULL)
		SpawnedParticle = UGameplayStatics::SpawnEmitterAttached(LinkParticle, Platform, LinkSocket);

}

void ARunPlatform_Physic::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (LinkParticle != NULL && SpawnedParticle != NULL)
		SpawnedParticle->SetBeamTargetPoint(0, AttachedMesh->GetComponentLocation() - FVector(0.f, 0.f, 50.f), 0);
}

void ARunPlatform_Physic::EndOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	Super::EndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
	SpawnedParticle->SetVisibility(false);
	AttachedMesh->SetSimulatePhysics(true);
	//ConstraintComponent->DestroyPhysicsState();	//取消约束
	ConstraintComponent->BreakConstraint();
}

