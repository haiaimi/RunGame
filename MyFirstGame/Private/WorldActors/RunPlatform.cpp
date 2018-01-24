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

	//����ײ��ֻ���ڼ��
	QueryBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	QueryBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	QueryBox->bGenerateOverlapEvents = true;       //������ײ�¼�

	QueryBox->SetupAttachment(Platform);
	Platform->SetupAttachment(ArrowDst);
	RootComponent = ArrowDst;

	IsSlope = false;    //Ĭ���ǲ���б��
	SlopeAngle = 60.f;
	IsShootToSlope = false; //Ĭ��������ƽ̨ģʽ

	PlatDir = EPlatformDirection::Absolute_Forward;  //Ĭ����ǰ
}

// Called when the game starts or when spawned
void ARunPlatform::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	QueryBox->OnComponentBeginOverlap.AddDynamic(this, &ARunPlatform::BeginOverlap);  //Ϊ��ײ�������ײ��Ӧ
	QueryBox->OnComponentEndOverlap.AddDynamic(this, &ARunPlatform::EndOverlap);   //��ײ����ʱ����Ӧ

	FVector PlatformSize = Platform->Bounds.BoxExtent;   //��ȡPlatform�Ĵ�С
	FVector QuerySize = QueryBox->Bounds.BoxExtent; //��ȡ��ײ��Ĵ�С
	FVector BoxScale = FVector(PlatformSize.X / QuerySize.X, PlatformSize.Y / QuerySize.Y, 15 * PlatformSize.Z / QuerySize.Z);
	QueryBox->SetWorldScale3D(BoxScale);      //����Platform��С������ײ��ߴ�
	QueryBox->SetRelativeLocation(FVector(PlatformSize.X, PlatformSize.Y, QueryBox->Bounds.BoxExtent.Z));  //Ȼ�����ü����ײ���λ��
	Platform->SetWorldScale3D(FVector(XScale, YScale, 1.f));

	GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Blue, FString::Printf(TEXT("X:%2f,Y:%2f,Z:%2f,Platform.X:%2f"), PlatformSize.X, PlatformSize.Y, BoxScale.Z, PlatformSize.X));
	
	PlatformLength = 2 * XScale * PlatformSize.X;
	PlatformWidth = 2 * YScale * PlatformSize.Y;
	//��ȡ��бʱĿ��Ƕ�
	DstRotation = GetActorRotation() - FRotator(SlopeAngle, 0.f, 0.f);
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
		
		//�����Ǹ�����ҵ�����ƶ��ٶȺͶ�������
		float rate = 1 - FMath::Abs(Temp.Pitch - DstRotation.Pitch) / SlopeAngle;
		InSlope(rate);
	}
}

void ARunPlatform::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (Cast<AMyFirstGameCharacter>(OtherActor))
	{
		CurChar = Cast<AMyFirstGameCharacter>(OtherActor);   //��ǰ��ƽ̨�ϵ����
		MaxAcclerateSpeed = CurChar->MaxAcclerateSpeed;
		MaxRunSpeed = CurChar->MaxRunSpeed;

		/*������п��������ƶ��ٶȣ���ģ�������µļ��٣�ע����϶���*/
		if (!IsShootToSlope)
		{
			IsSlope = true;      //����ƽ̨��б�𽥼����ٶ�
		}
		else
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
	if (Cast<AMyFirstGameCharacter>(OtherActor))
	{
		IsSlope = false;
		Platform->SetSimulatePhysics(true);  //��������ģ��
		if (CurChar)
		{
			//�ָ����ԭ�����ƶ��ٶ�,�Ͷ�����������
			CurChar->MaxAcclerateSpeed = MaxAcclerateSpeed;
			CurChar->MaxRunSpeed = MaxRunSpeed;
			CurChar->RunRate = 1.f;
			//GEngine->AddOnScreenDebugMessage(1, 5, FColor::Blue, FString::Printf(TEXT("MaxAcclerateSpeed:%f,  MaxRunSpeed:%f,  RunRate:%f"), CurChar->MaxAcclerateSpeed, CurChar->MaxRunSpeed, CurChar->RunRate));
		}
		QueryBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);  //����Ҫ�Ѹ�ƽ̨����ײ����ر�

		if (Cast<AMyPlayerController>(CurChar->Controller))
		{
			AMyPlayerController* MPC = Cast<AMyPlayerController>(CurChar->Controller);
			MPC->CurPlatform = NULL;     //��ҵ�ǰ����ƽ̨��Ϊ��
		}
		GetWorldTimerManager().SetTimer(DestoryHandle, this, &ARunPlatform::DestroyActor, 4.f, false);   //4���ɾ����ƽ̨���ͷ��ڴ�
	}
}

void ARunPlatform::DestroyActor()
{
	Super::Destroy();
	GetWorldTimerManager().ClearTimer(DestoryHandle);      //�����ʱ��
}

void ARunPlatform::InSlope(float rate)
{
	if (CurChar)
	{
		if (CurChar->IsInAccelerate)
		{
			CurChar->RunRate = 1.f - 0.4*rate;   //�������������Ϊԭ����0.6��̫�ͻᵼ�²�����
			CurChar->CurMaxAcclerateSpeed = MaxAcclerateSpeed - MaxAcclerateSpeed * 0.4f * rate;
		}
		else
		{
			CurChar->RunRate = 1.f - 0.4*rate;   //�������������Ϊԭ����0.6��̫�ͻᵼ�²�����
			CurChar->CurMaxRunSpeed = MaxRunSpeed - MaxRunSpeed * 0.4f * rate;
		}
	}
}
