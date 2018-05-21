// Fill out your copyright notice in the Description page of Project Settings.

#include "JumpPlatform.h"
#include "Components/StaticMeshComponent.h"
#include "MyFirstGameCharacter.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Components/BoxComponent.h"
#include "ConstructorHelpers.h"
#include "Curves/CurveFloat.h"

AJumpPlatform::AJumpPlatform(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	NoPlayerToSlope = true;     //需要玩家触发旋转
	XScale = 1.f;
	YScale = 1.f;
	SlopeAngle = 30.f;
	bCanMove = false;
	MoveCurveTime = 0.f;
	MoveStartTime = 0.f;

	static ConstructorHelpers::FObjectFinder<UCurveFloat> FindMoveCurve(TEXT("/Game/Blueprint/Curves/JumpPlatMoveCurve"));
	MoveCurve = FindMoveCurve.Object;
}

void AJumpPlatform::BeginPlay()
{
	Super::BeginPlay();

	bCanMove = true;
}

void AJumpPlatform::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SetActorRotation(SpawnRotation);
	DstRotation = FRotator(0.f, SpawnRotation.Yaw, 0.f);
}

void AJumpPlatform::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction & ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	MoveStartTime -= DeltaTime;
	if (bCanMove && MoveCurve && MoveStartTime <= 0.f)
	{
		FVector MoveDir = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Y).GetSafeNormal2D();
		float DeltaDistance = MoveCurve->GetFloatValue(MoveCurveTime) * 150.f;
		SetActorLocation(SpawnLocation + MoveDir * DeltaDistance);

		MoveCurveTime += DeltaTime;
		if (MoveCurveTime > 2.f)
			MoveCurveTime = 0.f;
	}

	if (CurChar)
	{
		CurChar->AddedSpeed = FMath::FInterpConstantTo(CurChar->AddedSpeed, 0.f, DeltaTime, 200.f);
	}
}

void AJumpPlatform::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (Cast<AMyFirstGameCharacter>(OtherActor))
	{
		AMyFirstGameCharacter* MGC = Cast<AMyFirstGameCharacter>(OtherActor);
		CurChar = MGC;     //设置当前平台上的玩家
		CurChar->AddedSpeed = 199.f;
		IsSlope = true;
		GetWorldTimerManager().SetTimer(ToFall, this, &AJumpPlatform::StartDestroy, 2.f);    //两秒后坠落
	}
}

void AJumpPlatform::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<AMyFirstGameCharacter>(OtherActor) && CurChar)
	{
		IsSlope = false;
		QueryBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);  //这里要把该平台的碰撞体检测关闭
		CurChar->AddedSpeed = 0.f;
		CurChar = nullptr;
	}
}

void AJumpPlatform::StartDestroy()
{
	Super::StartDestroy();

	//逐渐删除周围的跳跃平台
	if (PrePlatform != nullptr && !PrePlatform->IsInDestroyed)
	{
		PrePlatform->NextPlatform = nullptr;
		PrePlatform->StartDestroy();
	}

	if (NextPlatform != nullptr && !NextPlatform->IsInDestroyed)
	{
		AJumpPlatform* JumpPlat = Cast<AJumpPlatform>(NextPlatform);
		JumpPlat->PrePlatform = nullptr;

		FTimerHandle NextFall;
		FTimerDelegate Delegate;
		
		Delegate.BindLambda([=]() {JumpPlat->StartDestroy(); });   //绑定Lambda 匿名函数
		GetWorldTimerManager().SetTimer(NextFall, Delegate, 2.f, false);
	}
}
