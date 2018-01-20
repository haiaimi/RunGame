// Fill out your copyright notice in the Description page of Project Settings.

#include "RunPlatform.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "MyFirstGameCharacter.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

// Sets default values
ARunPlatform::ARunPlatform(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Platform = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Platform"));
	QueryBox = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("QueryBox"));
	ArrowDst = ObjectInitializer.CreateDefaultSubobject<UArrowComponent>(this, TEXT("Arrow"));

	//该碰撞体只用于检测
	QueryBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	QueryBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	QueryBox->bGenerateOverlapEvents = true;       //生成碰撞事件

	QueryBox->SetupAttachment(Platform);
	Platform->SetupAttachment(ArrowDst);
	RootComponent = ArrowDst;

	IsSlope = false;    //默认是不倾斜的
	XScale = 1.f;
	YScale = 1.f;
}

// Called when the game starts or when spawned
void ARunPlatform::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	QueryBox->OnComponentBeginOverlap.AddDynamic(this,&ARunPlatform::BeginOverlap);  //为碰撞体添加碰撞响应
	QueryBox->OnComponentEndOverlap.AddDynamic(this, &ARunPlatform::EndOverlap);   //碰撞结束时的响应

	XScale = 2.f;   //注意平台大小比例变化
	FVector PlatformSize = Platform->Bounds.BoxExtent;   //获取Platform的大小
	FVector QuerySize = QueryBox->Bounds.BoxExtent; //获取碰撞体的大小
	FVector BoxScale = FVector(XScale*PlatformSize.X / QuerySize.X, YScale*PlatformSize.Y / QuerySize.Y, 10 * PlatformSize.Z / QuerySize.Z);
	QueryBox->SetWorldScale3D(BoxScale);   //根据Platform大小设置碰撞体尺寸
	GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Blue, FString::Printf(TEXT("X:%2f,Y:%2f,Z:%2f,Platform.X:%2f"), BoxScale.X, BoxScale.Y, BoxScale.Z, PlatformSize.X));
	Platform->SetRelativeLocation(FVector(-PlatformSize.X * 2, 0.f, 0.f));   //先设置Platform相对于Arrow的相对位置
	QueryBox->SetRelativeLocation(FVector(PlatformSize.X / XScale, PlatformSize.Y / YScale, QueryBox->Bounds.BoxExtent.Z));  //然后设置检测碰撞体的位置，一定要除以缩放比例，貌似是按原图形的大小数据进行位移，然后按比例移动缩放后的图形

	//获取倾斜时目标角度
	DstRotation = GetActorRotation() - FRotator(60.f, 0.f, 0.f);
}

// Called every frame
void ARunPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsSlope)
	{
		FRotator Temp = FMath::RInterpConstantTo(GetActorRotation(), DstRotation, DeltaTime, 20.f);  //以每秒20度的速度倾斜
		SetActorRotation(Temp);

		if (GetActorRotation().Pitch <= -60.f)
			IsSlope = false;
	}
}

void ARunPlatform::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (Cast<AMyFirstGameCharacter>(OtherActor))
	{
		AMyFirstGameCharacter* CurChar = Cast<AMyFirstGameCharacter>(OtherActor);   //当前在平台上的玩家
		IsSlope = true;
	}
}

void ARunPlatform::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<AMyFirstGameCharacter>(OtherActor))
	{
		Platform->SetSimulatePhysics(true);  //开启物理模拟
		GetWorldTimerManager().SetTimer(DestoryHandle, this, &ARunPlatform::DestroyActor, 4.f, false);   //四秒后删除该平台，释放内存
	}
}

void ARunPlatform::DestroyActor()
{
	Super::Destroy();
}