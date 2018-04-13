// Fill out your copyright notice in the Description page of Project Settings.

#include "Bonus.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "MyFirstGameCharacter.h"
#include "RunPlatform.h"
#include "Engine/Engine.h"
#include "ConstructorHelpers.h"
#include "Player/MyPlayerController.h"
#include "SceneView.h"
#include "Engine/LocalPlayer.h"
#include "RunGameState.h"


// Sets default values
ABonus::ABonus(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BonusShape = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("BonusMesh"));
	BonusQuery = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("BonusQuery"));

	BonusShape->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BonusShape->SetCollisionEnabled(ECollisionEnabled::QueryOnly);     //�����ڼ��

	BonusQuery->SetCollisionObjectType(COLLISION_BOOMQUERY);
	BonusQuery->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BonusQuery->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);   //ֻ����Ҽ��

	RootComponent = BonusShape;
	BonusQuery->SetupAttachment(BonusShape);

	ControllerRef = nullptr;
	bISMoveToScoreBorder = false;
}

// Called when the game starts or when spawned
void ABonus::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//����ײ���Bonus����
	FVector ShapeExtent = BonusShape->Bounds.BoxExtent;
	FVector QueryExtent = BonusQuery->Bounds.BoxExtent;
	//������ײ���Χ��״
	BonusQuery->SetRelativeScale3D(FVector(ShapeExtent.X / QueryExtent.X, ShapeExtent.Y / QueryExtent.Y, ShapeExtent.Z / QueryExtent.Z));
	BonusQuery->OnComponentBeginOverlap.AddDynamic(this, &ABonus::BeginOverlap);    //�����ײ��Ӧ

	BonusShape->SetWorldScale3D(BonusData.ShapeScale);   //���ý�������Ĵ�С
	SetActorRotation(GetActorRotation() + BonusData.ShapeRotation);
}

// Called every frame
void ABonus::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	RotateStartTime -= DeltaTime;
	//��ת
	if (RotateStartTime <= 0)
		SetActorRotation(GetActorRotation() + FRotator(0.f, DeltaTime * 180.f, 0.f));
	if (bISMoveToScoreBorder)
		MoveToScoreBorder(DeltaTime);
}

void ABonus::BeginPlay()
{
	Super::BeginPlay();

	if (Cast<AMyPlayerController>(GetOwner()))
	{
		ControllerRef = Cast<AMyPlayerController>(GetOwner());
	}
}

void ABonus::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<AMyFirstGameCharacter>(OtherActor) && ControllerRef)
	{
		if (BonusData.BonusType == EBonusType::Bonus_Score)
			bISMoveToScoreBorder = true;
		else if (BonusData.BonusType == EBonusType::Bonus_NoObstacle)
		{
			AMyFirstGameCharacter* const MC = Cast<AMyFirstGameCharacter>(ControllerRef->GetPawn());
			MC->ApplyBonus(this);
			Destroy();
		}
	}
}

void ABonus::DestroyActor()
{
	Super::Destroy();
}

void ABonus::StartFall()
{
	this->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform); //������Actor����

	if (!BonusShape->IsSimulatingPhysics())
		BonusShape->SetSimulatePhysics(true);     //��������ģ��
}
void ABonus::MoveToScoreBorder(float DeltaTime)
{
	if (ControllerRef)
	{
		FVector WorldPosition, WorldDirection;
		FViewport* ViewPort = ControllerRef->GetLocalPlayer()->ViewportClient->Viewport;
		ControllerRef->GetLocalPlayer()->GetProjectionData(ViewPort, eSSP_FULL, ProjectionData);      //��ȡ��ǰ��ͶӰ����

		FVector2D ScreenPosition;
		ScreenPosition.X = ViewPort->GetSizeXY().X + ControllerRef->ScoreBorderRelativeToTopLeft.X;
		ScreenPosition.Y = ControllerRef->ScoreBorderRelativeToTopLeft.Y;
		FMatrix const InvViewProjMatrix = ProjectionData.ComputeViewProjectionMatrix().InverseFast();
		FSceneView::DeprojectScreenToWorld(ScreenPosition, ProjectionData.GetConstrainedViewRect(), InvViewProjMatrix, WorldPosition, WorldDirection);
		
		FVector NewLocation = FMath::VInterpConstantTo(GetActorLocation(), WorldPosition + WorldDirection * 500.f, DeltaTime, 2000.f);
		FVector NewScale = GetActorScale3D() - DeltaTime * FVector(0.2f, 0.2f, 0.2f);
		if (NewScale.X < 0 || NewScale.Y < 0 || NewScale.Z < 0)
			NewScale = FVector(0.01f, 0.01f, 0.01f);

		SetActorScale3D(NewScale);
		
		SetActorLocation(NewLocation);
		if (NewLocation == WorldPosition + WorldDirection * 500.f)
		{
			AMyFirstGameCharacter* const MC = Cast<AMyFirstGameCharacter>(ControllerRef->GetPawn());
			MC->ApplyBonus(this);
			ARunGameState* RGS = Cast<ARunGameState>(GetWorld()->GetGameState());
			if (RGS)
			{
				RGS->EmphasizeScoreDelegate.Broadcast();
			}
			Destroy();
		}
	}
}