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
#include "RunPlatform_Shoot.h"

// Sets default values
ARunPlatform::ARunPlatform(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Platform = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Platform"));
	QueryBox = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("QueryBox"));
	ArrowDst = ObjectInitializer.CreateDefaultSubobject<UArrowComponent>(this, TEXT("Arrow"));

	//该碰撞体只用于检测
	QueryBox->SetCollisionObjectType(COLLISION_BOOMQUERY);  
	QueryBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	QueryBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	QueryBox->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECollisionResponse::ECR_Ignore); //忽略子弹
	QueryBox->bGenerateOverlapEvents = true;       //生成碰撞事件

	QueryBox->SetupAttachment(Platform);
	Platform->SetupAttachment(ArrowDst);
	RootComponent = ArrowDst;

	IsSlope = false;    //默认是不倾斜的
	SlopeAngle = 60.f;
	NoPlayerToSlope = false; //默认是正常平台模式

	SafeStayTime = 0.3f;  //默认安全时间
	PlatDir = EPlatformDirection::Absolute_Forward;  //默认向前
	MoveToNew = false;
	IsInDestroyed = false;
	MoveToOrigin = false;
}

// Called when the game starts or when spawned
void ARunPlatform::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	QueryBox->OnComponentBeginOverlap.AddDynamic(this, &ARunPlatform::BeginOverlap);  //为碰撞体添加碰撞响应
	QueryBox->OnComponentEndOverlap.AddDynamic(this, &ARunPlatform::EndOverlap);   //碰撞结束时的响应

	const FVector PlatformSize = Platform->Bounds.BoxExtent;   //获取Platform的大小
	const FVector QuerySize = QueryBox->Bounds.BoxExtent; //获取碰撞体的大小
	const FVector BoxScale = FVector(PlatformSize.X / QuerySize.X, PlatformSize.Y / QuerySize.Y, 15 * PlatformSize.Z / QuerySize.Z);
	QueryBox->SetWorldScale3D(BoxScale);      //根据Platform大小设置碰撞体尺寸
	QueryBox->SetRelativeLocation(FVector(PlatformSize.X, PlatformSize.Y, QueryBox->Bounds.BoxExtent.Z));  //然后设置检测碰撞体的位置
	Platform->SetWorldScale3D(FVector(XScale, YScale, 1.f));

	//GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Blue, FString::Printf(TEXT("X:%2f,Y:%2f,Z:%2f,Platform.X:%2f"), PlatformSize.X, PlatformSize.Y, BoxScale.Z, PlatformSize.X));
	
	PlatformLength = 2 * XScale * PlatformSize.X;
	PlatformWidth = 2 * YScale * PlatformSize.Y;
	//获取倾斜时目标角度
	DstRotation = GetActorRotation() - FRotator(SlopeAngle, 0.f, 0.f);
	SpawnLocation = GetActorLocation();
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
		
		//下面是更新玩家的最大移动速度和动画比例，只有在是普通模式下才执行
		if (!NoPlayerToSlope)
		{
			if (SafeStayTime <= 0)
				SafeStayTime = 0.f;  //安全时间已过
			else
				SafeStayTime -= DeltaTime;

			float rate = 1 - FMath::Abs(Temp.Pitch - DstRotation.Pitch) / SlopeAngle;
			InSlope(rate);
		}
	}
	if (CurChar != nullptr && NoPlayerToSlope) //玩家在平台上（射击触发模式）
	{
		if (SafeStayTime <= 0)
			SafeStayTime = 0.f;  //安全时间已过
		else
			SafeStayTime -= DeltaTime;
	}

	if (MoveToNew && !MoveToAll && !MoveToOrigin)
		MoveTick(DeltaTime);

	if (MoveToAll && !MoveToNew && !MoveToOrigin)
		MoveToAllTick(DeltaTime);

	if (MoveToOrigin && !MoveToNew && !MoveToAll)
		MoveToOriginTick(DeltaTime);
}


void ARunPlatform::BeginPlay()
{
	Super::BeginPlay();

	//WorldPlayerController = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
}

void ARunPlatform::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (Cast<AMyFirstGameCharacter>(OtherActor))
	{
		CurChar = Cast<AMyFirstGameCharacter>(OtherActor);   //当前在平台上的玩家
		MaxAcclerateSpeed = CurChar->MaxAcclerateSpeed;
		MaxRunSpeed = CurChar->MaxRunSpeed;

		/*下面进行控制人物移动速度，来模拟人上坡的减速，注意配合动画*/
		if (!NoPlayerToSlope)
		{
			IsSlope = true;      //随着平台倾斜逐渐减低速度
		}
		else if (!NoPlayerToSlope && !IsToAll)                //在无障碍模式下不减速
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
	if (SafeStayTime == 0)   //在安全时间内不会触发该函数内容
	{
		if (Cast<AMyFirstGameCharacter>(OtherActor))
		{
			IsSlope = false;

			if (OnFall.IsBound())
				OnFall.Broadcast(); 

			if (CurChar && Cast<ARunPlatform_Shoot>(this->NextPlatform) == nullptr)  //只有下一个平台不是Shoot平台才会恢复移速
			{
				//恢复玩家原本的移动速度,和动画播放速率
				CurChar->CurMaxAcclerateSpeed = MaxAcclerateSpeed;
				CurChar->CurMaxRunSpeed = MaxRunSpeed;
				CurChar->RunRate = 1.f;
				//GEngine->AddOnScreenDebugMessage(1, 5, FColor::Blue, FString::Printf(TEXT("MaxAcclerateSpeed:%f,  MaxRunSpeed:%f,  RunRate:%f"), CurChar->MaxAcclerateSpeed, CurChar->MaxRunSpeed, CurChar->RunRate));
			}
			QueryBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);  //这里要把该平台的碰撞体检测关闭

			if (Cast<AMyPlayerController>(CurChar->Controller))
			{
				AMyPlayerController* MPC = Cast<AMyPlayerController>(CurChar->Controller);
				int32 FoundIndex;
				if (MPC != nullptr)
					if (!MPC->PlatformArray.Find(MPC->CurPlatform, FoundIndex))
						MPC->CurPlatform = nullptr;     //玩家当前所在平台设为空
			}
			StartDestroy();
		}
	}
}

void ARunPlatform::StartDestroy()
{
	IsInDestroyed = true;
	if (!Platform->IsSimulatingPhysics())
		Platform->SetSimulatePhysics(true);  //开启物理模拟

	GetWorldTimerManager().SetTimer(DestoryHandle, this, &ARunPlatform::DestroyActor, 4.f, false);   //4秒后删除该平台，释放内存

	if (FlyObstacleDestory.IsBound())
	{
		FlyObstacleDestory.Broadcast();
	}
}


void ARunPlatform::DestroyActor()
{
	if (DestoryHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(DestoryHandle);      //清除定时器
	}

	if (OnDestory.IsBound())
	{
		OnDestory.Broadcast();   //开始执行代理（含有多个Actor)
	}
	Super::Destroy();
}

void ARunPlatform::InSlope(float rate)
{
	if (CurChar)
	{
		CurChar->RunRate = 1.f - 0.4*rate;   //动作的最低速率为原来的0.6，太低会导致不连贯
		CurChar->CurMaxAcclerateSpeed = MaxAcclerateSpeed - MaxAcclerateSpeed * 0.4f * rate;
		CurChar->CurMaxRunSpeed = MaxRunSpeed - MaxRunSpeed * 0.4f * rate;
	}
}

void ARunPlatform::MoveToNewPos(const FVector DeltaDistance)
{
	MoveToNew = true;
	DeltaLoc = DeltaDistance;   //设置移动的相对距离
}

void ARunPlatform::MoveToAllFun(const FVector DeltaDistance)
{
	MoveToAll = true;
	IsToAll = true;
	NoPlayerToSlope = true;     //停止平台旋转
	DeltaLoc = DeltaDistance;

	/*if (CurChar != nullptr)
	{
		CurChar->CurMaxAcclerateSpeed = MaxAcclerateSpeed;
		CurChar->CurMaxRunSpeed = MaxRunSpeed;
	}*/
}

void ARunPlatform::StopToAllFun(const FVector DeltaDistance)
{
	MoveToOrigin = true;
	NoPlayerToSlope = false;
	IsToAll = false;
	DeltaLoc = DeltaDistance;
}

void ARunPlatform::MoveTick(float DeltaTime)
{
	const FVector NewPos = FMath::VInterpTo(GetActorLocation(), SpawnLocation + DeltaLoc, DeltaTime, 10.f);
	SetActorLocation(NewPos);

	if (NextPlatform != nullptr)
		if (!NextPlatform->MoveToNew)     //只有下一个平台没有移动时才执行下面操作
			if ((NewPos - SpawnLocation).Size() > DeltaLoc.Size() / 2)     //移动超过相差距离一半时，就开始移动下一个平台
				NextPlatform->MoveToNewPos(DeltaLoc);

	if ((NewPos - (SpawnLocation + DeltaLoc)).Size() < 1.f)
	{
		MoveToNew = false;
		SpawnLocation = NewPos;

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

void ARunPlatform::MoveToAllTick(float DeltaTime)
{
	const FVector NewPos = FMath::VInterpTo(GetActorLocation(), SpawnLocation + DeltaLoc, DeltaTime, 10.f);
	SetActorLocation(NewPos);

	if (NextPlatform)
		if (!NextPlatform->MoveToAll)
			if ((NewPos - SpawnLocation).Size() >= (DeltaLoc.Size() / 2))     //移动超过相差距离一半时，就开始移动下一个平台
			{
				const AMyPlayerController* MPC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
				if (MPC != nullptr)
				{
					if (NextPlatform->IsA(MPC->SpawnPlatform) && (this->IsA(MPC->SpawnPlatform) || this->IsA(MPC->SpawnPlatform_Shoot)))      //当下一个平台是普通平台，并且当前平台是普通平台或射击平台
					{
						NextPlatform->MoveToAllFun(DeltaLoc);
					}
					else
					{
						const FVector NextDeltaPos = SpawnLocation + DeltaLoc - (NextPlatform->GetActorLocation() + NextPlatform->GetPlatformLength() * GetActorRotation().Vector());
						NextPlatform->MoveToAllFun(NextDeltaPos);
					}
				}
			}

	if ((NewPos - (SpawnLocation + DeltaLoc)).Size() < 1.f)
	{
		SpawnLocation = NewPos;
		MoveToAll = false;
	}
}

void ARunPlatform::MoveToOriginTick(float DeltaTime)
{
	const FVector NewPos = FMath::VInterpTo(GetActorLocation(), SpawnLocation + DeltaLoc, DeltaTime, 10.f);
	SetActorLocation(NewPos);

	if (NextPlatform)
		if (!NextPlatform->MoveToOrigin)
			if ((NewPos - SpawnLocation).Size() >= (DeltaLoc.Size() / 2))
			{
				const AMyPlayerController* MPC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
				if ((this->IsA(MPC->SpawnPlatform) || this->IsA(MPC->SpawnPlatform_Beam) || this->IsA(MPC->SpawnPlatform_Shoot)) && NextPlatform->IsA(MPC->SpawnPlatform))
				{
					NextPlatform->StopToAllFun(DeltaLoc);
				}

				else
				{
					const FVector NextDeltaPos = DeltaLoc + NextPlatform->DeltaLocToPrePlat + NextPlatform->GetPlatformLength() * NextPlatform->GetActorRotation().Vector();
					NextPlatform->StopToAllFun(NextDeltaPos);
				}
			}

	if ((NewPos - (SpawnLocation + DeltaLoc)).Size() < 1.f)
	{
		SpawnLocation = NewPos;
		MoveToOrigin = false;
	}
}