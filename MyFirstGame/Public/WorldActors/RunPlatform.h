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

	ARunPlatform* NextPlatform;

	/**����ɾ��ƽ̨��ʱ����*/
	FTimerHandle DestoryHandle;

	/**�Ƿ�ʼ��б*/
	uint8 IsSlope : 1;

	/**��б�ĽǶȣ����ж����������б*/
	float SlopeAngle;

	/**�Ƿ���ͨ�����ʹ��ƽ̨��ת��ģʽ*/
	uint8 IsShootToSlope : 1;

	/**��б�����սǶ�*/
	FRotator DstRotation;

	/*����������ƽ̨�����ű���*/
	float XScale = 2;  //Ĭ�Ϻ���������

	float YScale = 1;

	/**�����������µ�����ٶ�*/
	float MaxAcclerateSpeed, MaxRunSpeed;

	float PlatformLength, PlatformWidth;

	/*ƽ̨�ľ��Է�������XΪǰ����YΪ����YΪ��*/
	uint8 PlatDir;

protected:
	// Called when the game starts or when spawned
	virtual void PostInitializeComponents()override;

public:	
	// Called every frame
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

public:
	/**��ҽ��뵱ǰPlatformִ�еĲ���*/
	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	/**����뿪��ǰƽ̨��ִ�еĲ�������������������ɵ�Platform�Լ�ɾ����ǰPlatform*/
	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/**����бʱ�����Ĳ���*/
	void InSlope(float rate);

	void DestroyActor();

	/**��ȡƽ̨�ĳ���*/
	FORCEINLINE float GetPlatformLength() { return PlatformLength; }

	FORCEINLINE float GetPlatformWidth() { return PlatformWidth; }
};
