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
	class AMyFirstGameCharacter* CurChar;

	/**ָ���ƽ̨����һ��ƽ̨��ָ��*/
	ARunPlatform* NextPlatform;

	/**����ɾ��ƽ̨��ʱ����*/
	FTimerHandle DestoryHandle;

	/**�Ƿ�ʼ��б*/
	uint8 IsSlope : 1;

	/**��б�ĽǶȣ����ж����������б*/
	float SlopeAngle;

	/**�Ƿ���Ҫ�����ת*/
	uint8 NoPlayerToSlope : 1;

	/**��б�����սǶ�*/
	FRotator DstRotation;

	/*����������ƽ̨�����ű���*/
	float XScale = 2;  //Ĭ�Ϻ���������

	float YScale = 1;

	/**�����������µ�����ٶ�*/
	float MaxAcclerateSpeed, MaxRunSpeed;

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

	/**��ʼ����ƽ̨��λ��*/
	FVector SpawnLocation;

	uint8 MoveToNew : 1;

	/**λ�Ƶ���Ծ���*/
	FVector DeltaLoc;

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
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/**����бʱ�����Ĳ���*/
	void InSlope(float rate);

	void StartDestroy();

	void DestroyActor();

	/**��ȡƽ̨�ĳ���*/
	FORCEINLINE float GetPlatformLength() { return PlatformLength; }

	FORCEINLINE float GetPlatformWidth() { return PlatformWidth; }

	FORCEINLINE UStaticMeshComponent* GetMesh() { return Platform; }

	/**�����µ�λ��*/
	virtual void MoveToNewPos(FVector DeltaDistance);

	virtual void MoveTick(float DeltaTime);
};
