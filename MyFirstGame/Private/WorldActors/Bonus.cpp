// Fill out your copyright notice in the Description page of Project Settings.

#include "Bonus.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "MyFirstGameCharacter.h"
#include "RunPlatform.h"
#include "Engine/Engine.h"


// Sets default values
ABonus::ABonus(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BonusShape = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("BonusMesh"));
	BonusQuery = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("BonusQuery"));

	BonusShape->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BonusShape->SetCollisionEnabled(ECollisionEnabled::QueryOnly);     //不用于检测

	BonusQuery->SetCollisionObjectType(COLLISION_BOOMQUERY);
	BonusQuery->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BonusQuery->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);   //只对玩家检测

	RootComponent = BonusShape;
	BonusQuery->SetupAttachment(BonusShape);
}

// Called when the game starts or when spawned
void ABonus::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//用碰撞体绑定Bonus网格
	FVector ShapeExtent = BonusShape->Bounds.BoxExtent;
	FVector QueryExtent = BonusQuery->Bounds.BoxExtent;
	//设置碰撞体包围形状
	BonusQuery->SetRelativeScale3D(FVector(ShapeExtent.X / QueryExtent.X, ShapeExtent.Y / QueryExtent.Y, ShapeExtent.Z / QueryExtent.Z));
	BonusQuery->OnComponentBeginOverlap.AddDynamic(this, &ABonus::BeginOverlap);    //添加碰撞响应

	BonusShape->SetWorldScale3D(BonusData.ShapeScale);   //设置奖励网格的大小
	SetActorRotation(GetActorRotation() + BonusData.ShapeRotation);
}

// Called every frame
void ABonus::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

}

void ABonus::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<AMyFirstGameCharacter>(OtherActor))
	{
		AMyFirstGameCharacter* const MC = Cast<AMyFirstGameCharacter>(OtherActor);
		MC->ApplyBonus(this);
		Destroy();  //销毁
	}
}

void ABonus::DestroyActor()
{
	Super::Destroy();
}

void ABonus::StartFall()
{
	this->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform); //从依赖Actor脱离

	BonusShape->SetSimulatePhysics(true);     //开启物理模拟
	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString("Start Fall"), true);
}