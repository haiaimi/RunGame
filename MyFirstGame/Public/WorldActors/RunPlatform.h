// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyFirstGame.h"
#include "RunPlatform.generated.h"

DECLARE_MULTICAST_DELEGATE(FPlatformDestory);
DECLARE_MULTICAST_DELEGATE(FPlatformFall);
DECLARE_MULTICAST_DELEGATE(FFlyObstacleDestory);

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
	class AMyFirstGameCharacter* CurChar = nullptr;

	/**指向该平台的下一个平台的指针*/
	ARunPlatform* NextPlatform = nullptr;

	/**用于删除平台的时间句柄*/
	FTimerHandle DestoryHandle;

	/**是否开始倾斜*/
	uint8 IsSlope : 1;

	/**是否在删除中*/
	uint32 IsInDestroyed : 1;

	/**倾斜的角度，会有多种情况的倾斜*/
	float SlopeAngle;

	/**是否 不需要玩家旋转*/
	uint8 NoPlayerToSlope : 1;

	/**倾斜的最终角度*/
	FRotator DstRotation;

	/*下面两个是平台的缩放比例*/
	float XScale = 2;  //默认横向是两倍

	float YScale = 1;

	/**玩家正常情况下的最大速度*/
	//float MaxAcclerateSpeed, MaxRunSpeed;

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

	FFlyObstacleDestory FlyObstacleDestory;

	/**当前绑定的飞行障碍的数目*/
	int32 CurBoundFlyObstacleNum = 0;

	/**初始生成平台的位置*/
	FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")   //便于调试
	uint8 MoveToNew : 1;

	/**移动到统一平台*/
	uint8 MoveToAll : 1;

	/**移到原本应在的位置*/
	uint8 MoveToOrigin : 1;

	/**是否在平台集合状态*/
	uint8 IsToAll : 1;

	/**位移的相对距离*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")   //便于调试
	FVector DeltaLoc;

	/**相对于前一个平台的偏移位置，用于恢复平台集合前的状态*/
	FVector DeltaLocToPrePlat;

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
	virtual void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/**在倾斜时所做的操作*/
	void InSlope(float rate);

	virtual void StartDestroy();

	void DestroyActor();

	/**获取平台的长度*/
	FORCEINLINE float GetPlatformLength() { return PlatformLength; }

	FORCEINLINE float GetPlatformWidth() { return PlatformWidth; }

	FORCEINLINE UStaticMeshComponent* GetMesh() { return Platform; }

	/**移向新的位置*/
	virtual void MoveToNewPos(const FVector DeltaDistance);

	virtual void MoveToAllFun(const FVector DeltaDistance);

	virtual void StopToAllFun(const FVector DeltaDistance);

	virtual void MoveTick(float DeltaTime);

	/**用于在奖励玩家所有平台合到一起*/
	virtual void MoveToAllTick(float DeltaTime);

	virtual void MoveToOriginTick(float DeltaTime);
};
