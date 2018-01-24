// Fill out your copyright notice in the Description page of Project Settings.

#include "RunPlatform.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFrameWork/CharacterMovementComponent.h"
#include "MyFirstGameCharacter.h"
#include "Player/MyPlayerController.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "MyFirstGame.h"

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
	SlopeAngle = 60.f;
	IsShootToSlope = false; //默认是正常平台模式

	PlatDir = EPlatformDirection::Absolute_Forward;  //默认向前
}

// Called when the game starts or when spawned
void ARunPlatform::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	QueryBox->OnComponentBeginOverlap.AddDynamic(this, &ARunPlatform::BeginOverlap);  //为碰撞体添加碰撞响应
	QueryBox->OnComponentEndOverlap.AddDynamic(this, &ARunPlatform::EndOverlap);   //碰撞结束时的响应

	FVector PlatformSize = Platform->Bounds.BoxExtent;   //获取Platform的大小
	FVector QuerySize = QueryBox->Bounds.BoxExtent; //获取碰撞体的大小
	FVector BoxScale = FVector(PlatformSize.X / QuerySize.X, PlatformSize.Y / QuerySize.Y, 15 * PlatformSize.Z / QuerySize.Z);
	QueryBox->SetWorldScale3D(BoxScale);      //根据Platform大小设置碰撞体尺寸
	QueryBox->SetRelativeLocation(FVector(PlatformSize.X, PlatformSize.Y, QueryBox->Bounds.BoxExtent.Z));  //然后设置检测碰撞体的位置
	Platform->SetWorldScale3D(FVector(XScale, YScale, 1.f));

	GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Blue, FString::Printf(TEXT("X:%2f,Y:%2f,Z:%2f,Platform.X:%2f"), PlatformSize.X, PlatformSize.Y, BoxScale.Z, PlatformSize.X));
	
	PlatformLength = 2 * XScale * PlatformSize.X;
	PlatformWidth = 2 * YScale * PlatformSize.Y;
	//获取倾斜时目标角度
	DstRotation = GetActorRotation() - FRotator(SlopeAngle, 0.f, 0.f);
}

// Called every frame
void ARunPlatform::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (IsSlope)
	{
		FRotator Temp = FMath::RInterpConstantTo(GetActorRotation(), DstRotation, DeltaTime, 20.f);  //以每秒20度的速度倾斜
		SetActorRotation(Temp);

		if (GetActorRotation().Pitch <= -SlopeAngle)
			IsSlope = false;
		
		//下面是更新玩家的最大移动速度和动画比例
		float rate = 1 - FMath::Abs(Temp.Pitch - DstRotation.Pitch) / SlopeAngle;
		InSlope(rate);
	}
}

void ARunPlatform::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (Cast<AMyFirstGameCharacter>(OtherActor))
	{
		CurChar = Cast<AMyFirstGameCharacter>(OtherActor);   //当前在平台上的玩家
		MaxAcclerateSpeed = CurChar->MaxAcclerateSpeed;
		MaxRunSpeed = CurChar->MaxRunSpeed;

		/*下面进行控制人物移动速度，来模拟人上坡的减速，注意配合动画*/
		if (!IsShootToSlope)
		{
			IsSlope = true;      //随着平台倾斜逐渐减低速度
		}
		else
		{
			InSlope(SlopeAngle / 60.f);   //斜坡直接降低人物的速度
		}

		if (Cast<AMyPlayerController>(CurChar->Controller))
		{
			AMyPlayerController* MPC = Cast<AMyPlayerController>(CurChar->Controller);
			MPC->CurPlatform = this;     //玩家当前所在平台
		}
	}
}

void ARunPlatform::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<AMyFirstGameCharacter>(OtherActor))
	{
		IsSlope = false;
		Platform->SetSimulatePhysics(true);  //开启物理模拟
		if (CurChar)
		{
			//恢复玩家原本的移动速度,和动画播放速率
			CurChar->MaxAcclerateSpeed = MaxAcclerateSpeed;
			CurChar->MaxRunSpeed = MaxRunSpeed;
			CurChar->RunRate = 1.f;
			//GEngine->AddOnScreenDebugMessage(1, 5, FColor::Blue, FString::Printf(TEXT("MaxAcclerateSpeed:%f,  MaxRunSpeed:%f,  RunRate:%f"), CurChar->MaxAcclerateSpeed, CurChar->MaxRunSpeed, CurChar->RunRate));
		}
		QueryBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);  //这里要把该平台的碰撞体检测关闭

		if (Cast<AMyPlayerController>(CurChar->Controller))
		{
			AMyPlayerController* MPC = Cast<AMyPlayerController>(CurChar->Controller);
			MPC->CurPlatform = NULL;     //玩家当前所在平台设为空
		}
		GetWorldTimerManager().SetTimer(DestoryHandle, this, &ARunPlatform::DestroyActor, 4.f, false);   //4秒后删除该平台，释放内存
	}
}

void ARunPlatform::DestroyActor()
{
	Super::Destroy();
	GetWorldTimerManager().ClearTimer(DestoryHandle);      //清除定时器
}

void ARunPlatform::InSlope(float rate)
{
	if (CurChar)
	{
		if (CurChar->IsInAccelerate)
		{
			CurChar->RunRate = 1.f - 0.4*rate;   //动作的最低速率为原来的0.6，太低会导致不连贯
			CurChar->CurMaxAcclerateSpeed = MaxAcclerateSpeed - MaxAcclerateSpeed * 0.4f * rate;
		}
		else
		{
			CurChar->RunRate = 1.f - 0.4*rate;   //动作的最低速率为原来的0.6，太低会导致不连贯
			CurChar->CurMaxRunSpeed = MaxRunSpeed - MaxRunSpeed * 0.4f * rate;
		}
	}
}
