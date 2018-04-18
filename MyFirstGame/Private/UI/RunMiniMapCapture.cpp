// Fill out your copyright notice in the Description page of Project Settings.

#include "RunMiniMapCapture.h"
#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"


ARunMiniMapCapture::ARunMiniMapCapture(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	GetCaptureComponent2D()->bCaptureEveryFrame = false;       //不进行没帧更新、
	PrimaryActorTick.bCanEverTick = true;
	CameraFOV = 90.f;
	MiniMapHeight = 256;
	MiniMapWidth = 256;
}


void ARunMiniMapCapture::BeginPlay()
{
	Super::BeginPlay();

	MiniMapRenderTarget = NewObject<UTextureRenderTarget2D>();
	MiniMapRenderTarget->InitAutoFormat(MiniMapWidth, MiniMapHeight);

	GetCaptureComponent2D()->TextureTarget = MiniMapRenderTarget;
	GetCaptureComponent2D()->FOVAngle = CameraFOV;
}

void ARunMiniMapCapture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARunMiniMapCapture::SetSceneCaptureLocation(FVector Loc)
{
	GetCaptureComponent2D()->SetWorldLocation(Loc);
}

void ARunMiniMapCapture::SetSceneCaptureRotation(FRotator Rot)
{
	GetCaptureComponent2D()->SetWorldRotation(Rot);
}