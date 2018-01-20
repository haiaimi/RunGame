// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RunPlatform.generated.h"

UCLASS()
class MYFIRSTGAME_API ARunPlatform : public AActor
{
	GENERATED_UCLASS_BODY()
	
public:	
	/**游戏中的平台*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Platform;

	/**用于检测玩家，并做出相应变化的碰撞体，只用于检测*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	class UBoxComponent* QueryBox;

	/**用箭头组件做根组件，便于旋转*/
	UPROPERTY(EditAnywhere, Category = "Platform")
	class UArrowComponent* ArrowDst;

	/**用于删除平台的时间句柄*/
	FTimerHandle DestoryHandle;

	/**是否开始倾斜*/
	uint8 IsSlope : 1;

	/**倾斜的最终角度*/
	FRotator DstRotation;

	/*下面两个是平台的缩放比例*/
	float XScale;

	float YScale;

protected:
	// Called when the game starts or when spawned
	virtual void PostInitializeComponents()override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	/**玩家进入当前Platform执行的操作*/
	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	/**玩家离开当前平台所执行的操作，如决定接下来生成的Platform以及删除当前Platform*/
	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/**在倾斜时所做的操作*/
	void InSlope();

	void DestroyActor();
};
