// Fill out your copyright notice in the Description page of Project Settings.

#include "RunMiniMapCapture.h"
#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "MyFirstGame.h"

const FRotator LeftDir(-90.f, -90.f, 0.f);
const FRotator ForwardDir(-90.f, 0.f, 0.f);
const FRotator RightDir(-90.f, 90.f, 0.f);

ARunMiniMapCapture::ARunMiniMapCapture(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	GetCaptureComponent2D()->bCaptureEveryFrame = false;       //不进行每帧更新，只有移动或加载的时候进行渲染
	GetCaptureComponent2D()->bCaptureOnMovement = true;

	PrimaryActorTick.bCanEverTick = true;
	CameraFOV = 90.f;
	MiniMapHeight = 256;
	MiniMapWidth = 256;
	CachedDir = EPlatformDirection::Absolute_Forward;
	bUpdateRot = false;
}


void ARunMiniMapCapture::BeginPlay()
{
	Super::BeginPlay();
	
	MiniMapRenderTarget = NewObject<UTextureRenderTarget2D>();
	MiniMapRenderTarget->InitAutoFormat(MiniMapWidth, MiniMapHeight);

	GetCaptureComponent2D()->TextureTarget = MiniMapRenderTarget;
	GetCaptureComponent2D()->FOVAngle = CameraFOV;
	RootComponent->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
}

void ARunMiniMapCapture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bUpdateRot)
	{
		static float CurLerpAlpha = 0.f;
		//FRotator NextRot = FMath::RInterpTo(RootComponent->GetComponentRotation(), DstRotator, DeltaTime, 10.f);
		//RootComponent->SetWorldRotation(NextRot);

		//if (FMath::Abs(NextRot.Yaw - DstRotator.Yaw) < 1 && FMath::Abs(NextRot.Roll - DstRotator.Roll) < 1)
		//{
		//	RootComponent->SetWorldRotation(NextRot);
		//	bUpdateRot = false;      //停止更新角度
		//}
		CurLerpAlpha += DeltaTime * 2.f;
		FQuat quat(RootComponent->GetComponentRotation());
		FQuat DstQuat(DstRotator);
		FRotator newRotator = FQuat::Slerp(quat, DstQuat, CurLerpAlpha).Rotator();     //圆形差值
		RootComponent->SetWorldRotation(newRotator);

		if (CurLerpAlpha > 1)
		{
			bUpdateRot = false;
			CurLerpAlpha = 0.f;
		}
		UE_LOG(LogRunGame, Log, TEXT("X:%f, Y:%f, Z:%f"), RootComponent->GetComponentRotation().Pitch, RootComponent->GetComponentRotation().Yaw, RootComponent->GetComponentRotation().Roll)
	}
}

void ARunMiniMapCapture::SetSceneCaptureLocation(FVector Loc)
{
	RootComponent->SetWorldLocation(Loc + FVector(0.f, 0.f, 1000.f));
}

void ARunMiniMapCapture::SetSceneCaptureDir(EPlatformDirection::Type CurDir)
{
	bUpdateRot = true;
	CachedDir = CurDir;

	switch (CurDir)
	{
	case EPlatformDirection::Absolute_Left:
		DstRotator = LeftDir;
		break;
	case EPlatformDirection::Absolute_Forward:
		DstRotator = ForwardDir;
		break;
	case EPlatformDirection::Absolute_Right:
		DstRotator = RightDir;
		break;
	default:
		break;
	}
}