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

	//����ײ��ֻ���ڼ��
	QueryBox->SetCollisionObjectType(COLLISION_BOOMQUERY);  
	QueryBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	QueryBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	QueryBox->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECollisionResponse::ECR_Ignore); //�����ӵ�
	QueryBox->bGenerateOverlapEvents = true;       //������ײ�¼�

	QueryBox->SetupAttachment(Platform);
	Platform->SetupAttachment(ArrowDst);
	RootComponent = ArrowDst;

	IsSlope = false;    //Ĭ���ǲ���б��
	SlopeAngle = 60.f;
	NoPlayerToSlope = false; //Ĭ��������ƽ̨ģʽ

	SafeStayTime = 0.3f;  //Ĭ�ϰ�ȫʱ��
	PlatDir = EPlatformDirection::Absolute_Forward;  //Ĭ����ǰ
	MoveToNew = false;
	IsInDestroyed = false;
	MoveToOrigin = false;
}

// Called when the game starts or when spawned
void ARunPlatform::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	QueryBox->OnComponentBeginOverlap.AddDynamic(this, &ARunPlatform::BeginOverlap);  //Ϊ��ײ�������ײ��Ӧ
	QueryBox->OnComponentEndOverlap.AddDynamic(this, &ARunPlatform::EndOverlap);   //��ײ����ʱ����Ӧ

	const FVector PlatformSize = Platform->Bounds.BoxExtent;   //��ȡPlatform�Ĵ�С
	const FVector QuerySize = QueryBox->Bounds.BoxExtent; //��ȡ��ײ��Ĵ�С
	const FVector BoxScale = FVector(PlatformSize.X / QuerySize.X, PlatformSize.Y / QuerySize.Y, 15 * PlatformSize.Z / QuerySize.Z);
	QueryBox->SetWorldScale3D(BoxScale);      //����Platform��С������ײ��ߴ�
	QueryBox->SetRelativeLocation(FVector(PlatformSize.X, PlatformSize.Y, QueryBox->Bounds.BoxExtent.Z));  //Ȼ�����ü����ײ���λ��
	Platform->SetWorldScale3D(FVector(XScale, YScale, 1.f));

	//GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Blue, FString::Printf(TEXT("X:%2f,Y:%2f,Z:%2f,Platform.X:%2f"), PlatformSize.X, PlatformSize.Y, BoxScale.Z, PlatformSize.X));
	
	PlatformLength = 2 * XScale * PlatformSize.X;
	PlatformWidth = 2 * YScale * PlatformSize.Y;
	//��ȡ��бʱĿ��Ƕ�
	DstRotation = GetActorRotation() - FRotator(SlopeAngle, 0.f, 0.f);
	SpawnLocation = GetActorLocation();
}

// Called every frame
void ARunPlatform::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (IsSlope)
	{
		FRotator Temp = FMath::RInterpConstantTo(GetActorRotation(), DstRotation, DeltaTime, 20.f);  //��ÿ��20�ȵ��ٶ���б
		SetActorRotation(Temp);

		if (GetActorRotation().Pitch <= -SlopeAngle)
			IsSlope = false;
		
		//�����Ǹ�����ҵ�����ƶ��ٶȺͶ���������ֻ��������ͨģʽ�²�ִ��
		if (!NoPlayerToSlope)
		{
			if (SafeStayTime <= 0)
				SafeStayTime = 0.f;  //��ȫʱ���ѹ�
			else
				SafeStayTime -= DeltaTime;

			float rate = 1 - FMath::Abs(Temp.Pitch - DstRotation.Pitch) / SlopeAngle;
			InSlope(rate);
		}
	}
	if (CurChar != nullptr && NoPlayerToSlope) //�����ƽ̨�ϣ��������ģʽ��
	{
		if (SafeStayTime <= 0)
			SafeStayTime = 0.f;  //��ȫʱ���ѹ�
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
		CurChar = Cast<AMyFirstGameCharacter>(OtherActor);   //��ǰ��ƽ̨�ϵ����
		MaxAcclerateSpeed = CurChar->MaxAcclerateSpeed;
		MaxRunSpeed = CurChar->MaxRunSpeed;

		/*������п��������ƶ��ٶȣ���ģ�������µļ��٣�ע����϶���*/
		if (!NoPlayerToSlope)
		{
			IsSlope = true;      //����ƽ̨��б�𽥼����ٶ�
		}
		else if (!NoPlayerToSlope && !IsToAll)                //�����ϰ�ģʽ�²�����
		{
			InSlope(SlopeAngle / 60.f);   //б��ֱ�ӽ���������ٶ�
		}

		if (Cast<AMyPlayerController>(CurChar->Controller))
		{
			AMyPlayerController* MPC = Cast<AMyPlayerController>(CurChar->Controller);
			MPC->CurPlatform = this;     //��ҵ�ǰ����ƽ̨
		}
	}
}

void ARunPlatform::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (SafeStayTime == 0)   //�ڰ�ȫʱ���ڲ��ᴥ���ú�������
	{
		if (Cast<AMyFirstGameCharacter>(OtherActor))
		{
			IsSlope = false;

			if (OnFall.IsBound())
				OnFall.Broadcast(); 

			if (CurChar && Cast<ARunPlatform_Shoot>(this->NextPlatform) == nullptr)  //ֻ����һ��ƽ̨����Shootƽ̨�Ż�ָ�����
			{
				//�ָ����ԭ�����ƶ��ٶ�,�Ͷ�����������
				CurChar->CurMaxAcclerateSpeed = MaxAcclerateSpeed;
				CurChar->CurMaxRunSpeed = MaxRunSpeed;
				CurChar->RunRate = 1.f;
				//GEngine->AddOnScreenDebugMessage(1, 5, FColor::Blue, FString::Printf(TEXT("MaxAcclerateSpeed:%f,  MaxRunSpeed:%f,  RunRate:%f"), CurChar->MaxAcclerateSpeed, CurChar->MaxRunSpeed, CurChar->RunRate));
			}
			QueryBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);  //����Ҫ�Ѹ�ƽ̨����ײ����ر�

			if (Cast<AMyPlayerController>(CurChar->Controller))
			{
				AMyPlayerController* MPC = Cast<AMyPlayerController>(CurChar->Controller);
				int32 FoundIndex;
				if (MPC != nullptr)
					if (!MPC->PlatformArray.Find(MPC->CurPlatform, FoundIndex))
						MPC->CurPlatform = nullptr;     //��ҵ�ǰ����ƽ̨��Ϊ��
			}
			StartDestroy();
		}
	}
}

void ARunPlatform::StartDestroy()
{
	IsInDestroyed = true;
	if (!Platform->IsSimulatingPhysics())
		Platform->SetSimulatePhysics(true);  //��������ģ��

	GetWorldTimerManager().SetTimer(DestoryHandle, this, &ARunPlatform::DestroyActor, 4.f, false);   //4���ɾ����ƽ̨���ͷ��ڴ�

	if (FlyObstacleDestory.IsBound())
	{
		FlyObstacleDestory.Broadcast();
	}
}


void ARunPlatform::DestroyActor()
{
	if (DestoryHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(DestoryHandle);      //�����ʱ��
	}

	if (OnDestory.IsBound())
	{
		OnDestory.Broadcast();   //��ʼִ�д������ж��Actor)
	}
	Super::Destroy();
}

void ARunPlatform::InSlope(float rate)
{
	if (CurChar)
	{
		CurChar->RunRate = 1.f - 0.4*rate;   //�������������Ϊԭ����0.6��̫�ͻᵼ�²�����
		CurChar->CurMaxAcclerateSpeed = MaxAcclerateSpeed - MaxAcclerateSpeed * 0.4f * rate;
		CurChar->CurMaxRunSpeed = MaxRunSpeed - MaxRunSpeed * 0.4f * rate;
	}
}

void ARunPlatform::MoveToNewPos(const FVector DeltaDistance)
{
	MoveToNew = true;
	DeltaLoc = DeltaDistance;   //�����ƶ�����Ծ���
}

void ARunPlatform::MoveToAllFun(const FVector DeltaDistance)
{
	MoveToAll = true;
	IsToAll = true;
	NoPlayerToSlope = true;     //ֹͣƽ̨��ת
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
		if (!NextPlatform->MoveToNew)     //ֻ����һ��ƽ̨û���ƶ�ʱ��ִ���������
			if ((NewPos - SpawnLocation).Size() > DeltaLoc.Size() / 2)     //�ƶ�����������һ��ʱ���Ϳ�ʼ�ƶ���һ��ƽ̨
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
			if ((NewPos - SpawnLocation).Size() >= (DeltaLoc.Size() / 2))     //�ƶ�����������һ��ʱ���Ϳ�ʼ�ƶ���һ��ƽ̨
			{
				const AMyPlayerController* MPC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
				if (MPC != nullptr)
				{
					if (NextPlatform->IsA(MPC->SpawnPlatform) && (this->IsA(MPC->SpawnPlatform) || this->IsA(MPC->SpawnPlatform_Shoot)))      //����һ��ƽ̨����ͨƽ̨�����ҵ�ǰƽ̨����ͨƽ̨�����ƽ̨
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