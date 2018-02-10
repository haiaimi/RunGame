// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyFirstGame.h"
#include "RunPlatform.generated.h"

DECLARE_MULTICAST_DELEGATE(FPlatformDestory);
DECLARE_MULTICAST_DELEGATE(FPlatformFall);

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

	/**指向该平台的下一个平台的指针*/
	ARunPlatform* NextPlatform;

	/**用于删除平台的时间句柄*/
	FTimerHandle DestoryHandle;

	/**是否开始倾斜*/
	uint8 IsSlope : 1;

	/**倾斜的角度，会有多种情况的倾斜*/
	float SlopeAngle;

	/**是否不需要玩家旋转*/
	uint8 NoPlayerToSlope : 1;

	/**倾斜的最终角度*/
	FRotator DstRotation;

	/*下面两个是平台的缩放比例*/
	float XScale = 2;  //默认横向是两倍

	float YScale = 1;

	/**玩家正常情况下的最大速度*/
	float MaxAcclerateSpeed, MaxRunSpeed;

	float PlatformLength, PlatformWidth;

	/**玩家在平台上活动的安全时间，不会无意跳出碰撞体后造成平台坠落*/
	float SafeStayTime;

	/**平台的绝对方向，以正X为前，负Y为左，正Y为右*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform")
	TEnumAsByte<EPlatformDirection::Type> PlatDir;

	/**此多播代理用于在其上面的奖励Actor销毁*/
	FPlatformDestory OnDestory;

	/***/
	FPlatformFall OnFall;

	/**初始生成平台的位置*/
	FVector SpawnLocation;

	uint8 MoveToNew : 1;

	/**位移的相对距离*/
	FVector DeltaLoc;

protected:
	// Called when the game starts or when spawned
	virtual void PostInitializeComponents()override;

public:	
	// Called every frame
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	virtual void BeginPlay()override;

public:
	/**玩家进入当前Platform执行的操作*/
	UFUNCTION()
	virtual void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	/**玩家离开当前平台所执行的操作，如决定接下来生成的Platform以及删除当前Platform*/
	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/**在倾斜时所做的操作*/
	void InSlope(float rate);

	void StartDestroy();

	void DestroyActor();

	/**获取平台的长度*/
	FORCEINLINE float GetPlatformLength() { return PlatformLength; }

	FORCEINLINE float GetPlatformWidth() { return PlatformWidth; }

	FORCEINLINE UStaticMeshComponent* GetMesh() { return Platform; }

	/**移向新的位置*/
	virtual void MoveToNewPos(FVector DeltaDistance);

	virtual void MoveTick(float DeltaTime);
};
