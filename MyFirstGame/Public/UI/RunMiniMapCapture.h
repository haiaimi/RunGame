// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/SceneCapture2D.h"
#include "MyFirstGame.h"
#include "RunMiniMapCapture.generated.h"

/**
 *  捕捉小地图画面
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

	/**捕获摄像机的FOV*/
	float CameraFOV;

	/**摄像机位置*/
	FVector CameraLoc;

	class UTextureRenderTarget2D* MiniMapRenderTarget;

	TEnumAsByte<EPlatformDirection::Type> CachedDir;

	/**镜头旋转的目标角度*/
	FRotator DstRotator;

	/**该值会用于HUD中人物方向*/
	float RotatorYaw;

	float CachedRotatorYaw;

	uint32 bUpdateRot : 1;
	
public:
	/**设置捕获相机的位置*/
	void SetSceneCaptureLocation(FVector Loc);

	void SetSceneCaptureDir(EPlatformDirection::Type CurDir);
};
