// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/SceneCapture2D.h"
#include "RunMiniMapCapture.generated.h"

/**
 *  ��׽С��ͼ����
 */
UCLASS()
class MYFIRSTGAME_API ARunMiniMapCapture : public ASceneCapture2D
{
	GENERATED_UCLASS_BODY()

public:
	virtual void Tick(float DeltaTime)override;

	virtual void BeginPlay()override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MiniMap, meta = (ClampMin = "0", ClampMax = "1024"))
	int32 MiniMapWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MiniMap, meta = (ClampMin = "0", ClampMax = "1024"))
	int32 MiniMapHeight;

	/**�����������FOV*/
	float CameraFOV;

	/**�����λ��*/
	FVector CameraLoc;

	class UTextureRenderTarget2D* MiniMapRenderTarget;
	
public:
	/**���ò��������λ��*/
	void SetSceneCaptureLocation(FVector Loc);

	void SetSceneCaptureRotation(FRotator Rot);
};
