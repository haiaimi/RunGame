// Fill out your copyright notice in the Description page of Project Settings.

#include "RunPlatform.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFrameWork/CharacterMovementComponent.h"
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

	XScale = 2.f;    //注意平台大小比例变化
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
		
		//下面是更新玩家的最大移动速度和动画比例
		float rate = 1 - FMath::Abs(Temp.Pitch - DstRotation.Pitch) / 60.f;
		InSlope(rate);
	}
}

void ARunPlatform::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (Cast<AMyFirstGameCharacter>(OtherActor))
	{
		CurChar = Cast<AMyFirstGameCharacter>(OtherActor);   //当前在平台上的玩家
		IsSlope = true;

		/*下面进行控制人物移动速度，来模拟人上坡的减速，注意配合动画*/
		MaxAcclerateSpeed = CurChar->MaxAcclerateSpeed;
		MaxRunSpeed = CurChar->MaxRunSpeed;
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
		GetWorldTimerManager().SetTimer(DestoryHandle, this, &ARunPlatform::DestroyActor, 4.f, false);   //4秒后删除该平台，释放内存
	}
}

void ARunPlatform::DestroyActor()
{
	Super::Destroy();
}

void ARunPlatform::InSlope(float rate)
{
	if (CurChar)
	{
		if (CurChar->IsInAccelerate)
		{
			CurChar->RunRate = 1.f - 0.4*rate;   //动作的最低速率为原来的0.6，太低会导致不连贯
			CurChar->GetCharacterMovement()->MaxWalkSpeed = MaxAcclerateSpeed - MaxAcclerateSpeed * 0.4f*rate; //同样移速的最低也是原来的0.6
			CurChar->MaxAcclerateSpeed = MaxAcclerateSpeed - MaxAcclerateSpeed * 0.4f*rate;;
		}
		else
		{
			CurChar->RunRate = 1.f - 0.4*rate;   //动作的最低速率为原来的0.6，太低会导致不连贯
			CurChar->GetCharacterMovement()->MaxWalkSpeed = MaxRunSpeed - MaxRunSpeed * 0.4f*rate; //同样移速的最低也是原来的0.6
			CurChar->MaxRunSpeed = MaxRunSpeed - MaxRunSpeed * 0.4f*rate;;
		}
	}
}
