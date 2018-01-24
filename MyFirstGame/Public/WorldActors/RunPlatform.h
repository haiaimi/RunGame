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

	/**指向当前平台上的玩家*/
	class AMyFirstGameCharacter* CurChar;

	ARunPlatform* NextPlatform;

	/**用于删除平台的时间句柄*/
	FTimerHandle DestoryHandle;

	/**是否开始倾斜*/
	uint8 IsSlope : 1;

	/**倾斜的角度，会有多种情况的倾斜*/
	float SlopeAngle;

	/**是否是通过射击使得平台旋转的模式*/
	uint8 IsShootToSlope : 1;

	/**倾斜的最终角度*/
	FRotator DstRotation;

	/*下面两个是平台的缩放比例*/
	float XScale = 2;  //默认横向是两倍

	float YScale = 1;

	/**玩家正常情况下的最大速度*/
	float MaxAcclerateSpeed, MaxRunSpeed;

	float PlatformLength, PlatformWidth;

	/*平台的绝对方向，以正X为前，负Y为左，正Y为右*/
	uint8 PlatDir;

protected:
	// Called when the game starts or when spawned
	virtual void PostInitializeComponents()override;

public:	
	// Called every frame
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

public:
	/**玩家进入当前Platform执行的操作*/
	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	/**玩家离开当前平台所执行的操作，如决定接下来生成的Platform以及删除当前Platform*/
	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/**在倾斜时所做的操作*/
	void InSlope(float rate);

	void DestroyActor();

	/**获取平台的长度*/
	FORCEINLINE float GetPlatformLength() { return PlatformLength; }

	FORCEINLINE float GetPlatformWidth() { return PlatformWidth; }
};
