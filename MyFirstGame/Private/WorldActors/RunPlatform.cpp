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

	//����ײ��ֻ���ڼ��
	QueryBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	QueryBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	QueryBox->bGenerateOverlapEvents = true;       //������ײ�¼�

	QueryBox->SetupAttachment(Platform);
	Platform->SetupAttachment(ArrowDst);
	RootComponent = ArrowDst;

	IsSlope = false;    //Ĭ���ǲ���б��
	XScale = 1.f;
	YScale = 1.f;
}

// Called when the game starts or when spawned
void ARunPlatform::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	QueryBox->OnComponentBeginOverlap.AddDynamic(this,&ARunPlatform::BeginOverlap);  //Ϊ��ײ�������ײ��Ӧ
	QueryBox->OnComponentEndOverlap.AddDynamic(this, &ARunPlatform::EndOverlap);   //��ײ����ʱ����Ӧ

	XScale = 2.f;   //ע��ƽ̨��С�����仯
	FVector PlatformSize = Platform->Bounds.BoxExtent;   //��ȡPlatform�Ĵ�С
	FVector QuerySize = QueryBox->Bounds.BoxExtent; //��ȡ��ײ��Ĵ�С
	FVector BoxScale = FVector(XScale*PlatformSize.X / QuerySize.X, YScale*PlatformSize.Y / QuerySize.Y, 10 * PlatformSize.Z / QuerySize.Z);
	QueryBox->SetWorldScale3D(BoxScale);   //����Platform��С������ײ��ߴ�
	GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Blue, FString::Printf(TEXT("X:%2f,Y:%2f,Z:%2f,Platform.X:%2f"), BoxScale.X, BoxScale.Y, BoxScale.Z, PlatformSize.X));
	Platform->SetRelativeLocation(FVector(-PlatformSize.X * 2, 0.f, 0.f));   //������Platform�����Arrow�����λ��
	QueryBox->SetRelativeLocation(FVector(PlatformSize.X / XScale, PlatformSize.Y / YScale, QueryBox->Bounds.BoxExtent.Z));  //Ȼ�����ü����ײ���λ�ã�һ��Ҫ�������ű�����ò���ǰ�ԭͼ�εĴ�С���ݽ���λ�ƣ�Ȼ�󰴱����ƶ����ź��ͼ��

	//��ȡ��бʱĿ��Ƕ�
	DstRotation = GetActorRotation() - FRotator(60.f, 0.f, 0.f);
}

// Called every frame
void ARunPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsSlope)
	{
		FRotator Temp = FMath::RInterpConstantTo(GetActorRotation(), DstRotation, DeltaTime, 20.f);  //��ÿ��20�ȵ��ٶ���б
		SetActorRotation(Temp);

		if (GetActorRotation().Pitch <= -60.f)
			IsSlope = false;
	}
}

void ARunPlatform::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (Cast<AMyFirstGameCharacter>(OtherActor))
	{
		AMyFirstGameCharacter* CurChar = Cast<AMyFirstGameCharacter>(OtherActor);   //��ǰ��ƽ̨�ϵ����
		IsSlope = true;
	}
}

void ARunPlatform::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<AMyFirstGameCharacter>(OtherActor))
	{
		Platform->SetSimulatePhysics(true);  //��������ģ��
		GetWorldTimerManager().SetTimer(DestoryHandle, this, &ARunPlatform::DestroyActor, 4.f, false);   //�����ɾ����ƽ̨���ͷ��ڴ�
	}
}

void ARunPlatform::DestroyActor()
{
	Super::Destroy();
}