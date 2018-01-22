// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "MyPlayerCameraManager.h"
#include "EngineUtils.h"
#include "RunPlatform.h"
#include "ConstructorHelpers.h"
#include "Engine/Engine.h"

AMyPlayerController::AMyPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PlayerCameraManagerClass = AMyPlayerCameraManager::StaticClass();
	static ConstructorHelpers::FClassFinder<ARunPlatform> SpawnPlat(TEXT("/Game/Blueprint/RunPlatform_BP"));
	SpawnPlatform = SpawnPlat.Class;
}


void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void AMyPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	//PlatformArray.SetNum(10);       //ƽ̨���������Ϊ10

	//��ȡ��Ϸ������Ԥ���һ��ƽ̨����Ĭ�ϵ�һ����
	for (TActorIterator<ARunPlatform> It(GetWorld()); It; ++It)
	{
		CurPlatform = *It;
		//PlatformArray[0] = CurPlatform; //����ĵ�һ������Ĭ��ƽ̨
		GEngine->AddOnScreenDebugMessage(1, 5, FColor::Black, TEXT("�����ɹ�"));
	}

	FVector CurLocation = CurPlatform->GetActorLocation();
	FRotator CurRotation = CurPlatform->GetActorRotation();

	FMatrix RotatMatrix = FRotationMatrix(CurRotation);
	FVector ForwardDir = -RotatMatrix.GetUnitAxis(EAxis::X);
	FVector LeftDir = RotatMatrix.GetUnitAxis(EAxis::Y);
	FVector RightDir = -RotatMatrix.GetUnitAxis(EAxis::Y);

	FVector SpawnLocation = CurLocation + ForwardDir * CurPlatform->GetPlatformWidth() + LeftDir * CurPlatform->GetPlatformLength();
	FRotator SpawnRotation = CurRotation - FRotator(0.f, 90.f, 0.f);

	FTransform SpawnTransform = FTransform(SpawnRotation, SpawnLocation);
	GetWorld()->SpawnActor<ARunPlatform>(SpawnPlatform, SpawnTransform);

	////������������10��ƽ̨
	//int32 ArrayNum = PlatformArray.Num();
	//for (int32 i = 1; i < ArrayNum; i++)
	//{
	//	//ARunPlatform* Temp = GetWorld()->SpawnActor<ARunPlatform>(SpawnPlatform);
	//}
}

void AMyPlayerController::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
}

void AMyPlayerController::AutoSpawnPlatform()
{
	
}

//FTransform AMyPlayerController::GetRandomSpawnTransf(ARunPlatform* PrePlatform)
//{
//	if (PrePlatform)
//	{
//		int32 Dir = FMath::Rand() % 3;  //�����������
//
//		switch (Dir)
//		{
//			case 0: break;
//			
//		}
//	}
//	return FTransform();
//}