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
	/**��Ϸ�е�ƽ̨*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Platform;

	/**���ڼ����ң���������Ӧ�仯����ײ�壬ֻ���ڼ��*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	class UBoxComponent* QueryBox;

	/**�ü�ͷ������������������ת*/
	UPROPERTY(EditAnywhere, Category = "Platform")
	class UArrowComponent* ArrowDst;

	/**ָ��ǰƽ̨�ϵ����*/
	class AMyFirstGameCharacter* CurChar = nullptr;

	/**ָ���ƽ̨����һ��ƽ̨��ָ��*/
	ARunPlatform* NextPlatform = nullptr;

	/**����ɾ��ƽ̨��ʱ����*/
	FTimerHandle DestoryHandle;

	/**�Ƿ�ʼ��б*/
	uint8 IsSlope : 1;

	/**�Ƿ���ɾ����*/
	uint32 IsInDestroyed : 1;

	/**��б�ĽǶȣ����ж����������б*/
	float SlopeAngle;

	/**�Ƿ� ����Ҫ�����ת*/
	uint8 NoPlayerToSlope : 1;

	/**��б�����սǶ�*/
	FRotator DstRotation;

	/*����������ƽ̨�����ű���*/
	float XScale = 2;  //Ĭ�Ϻ���������

	float YScale = 1;

	/**�����������µ�����ٶ�*/
	//float MaxAcclerateSpeed, MaxRunSpeed;

	float PlatformLength, PlatformWidth;

	/**�����ƽ̨�ϻ�İ�ȫʱ�䣬��������������ײ������ƽ̨׹��*/
	float SafeStayTime;

	/**ƽ̨�ľ��Է�������XΪǰ����YΪ����YΪ��*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform")
	TEnumAsByte<EPlatformDirection::Type> PlatDir;

	/**�˶ಥ����������������Ľ���Actor����*/
	FPlatformDestory OnDestory;

	/***/
	FPlatformFall OnFall;

	FFlyObstacleDestory FlyObstacleDestory;

	/**��ǰ�󶨵ķ����ϰ�����Ŀ*/
	int32 CurBoundFlyObstacleNum = 0;

	/**��ʼ����ƽ̨��λ��*/
	FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")   //���ڵ���
	uint8 MoveToNew : 1;

	/**�ƶ���ͳһƽ̨*/
	uint8 MoveToAll : 1;

	/**�Ƶ�ԭ��Ӧ�ڵ�λ��*/
	uint8 MoveToOrigin : 1;

	/**�Ƿ���ƽ̨����״̬*/
	uint8 IsToAll : 1;

	/**λ�Ƶ���Ծ���*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")   //���ڵ���
	FVector DeltaLoc;

	/**�����ǰһ��ƽ̨��ƫ��λ�ã����ڻָ�ƽ̨����ǰ��״̬*/
	FVector DeltaLocToPrePlat;

protected:
	// Called when the game starts or when spawned
	virtual void PostInitializeComponents()override;

public:	
	// Called every frame
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	virtual void BeginPlay()override;

public:
	/**��ҽ��뵱ǰPlatformִ�еĲ���*/
	UFUNCTION()
	virtual void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	/**����뿪��ǰƽ̨��ִ�еĲ�������������������ɵ�Platform�Լ�ɾ����ǰPlatform*/
	UFUNCTION()
	virtual void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/**����бʱ�����Ĳ���*/
	void InSlope(float rate);

	virtual void StartDestroy();

	void DestroyActor();

	/**��ȡƽ̨�ĳ���*/
	FORCEINLINE float GetPlatformLength() { return PlatformLength; }

	FORCEINLINE float GetPlatformWidth() { return PlatformWidth; }

	FORCEINLINE UStaticMeshComponent* GetMesh() { return Platform; }

	/**�����µ�λ��*/
	virtual void MoveToNewPos(const FVector DeltaDistance);

	virtual void MoveToAllFun(const FVector DeltaDistance);

	virtual void StopToAllFun(const FVector DeltaDistance);

	virtual void MoveTick(float DeltaTime);

	/**�����ڽ����������ƽ̨�ϵ�һ��*/
	virtual void MoveToAllTick(float DeltaTime);

	virtual void MoveToOriginTick(float DeltaTime);
};
