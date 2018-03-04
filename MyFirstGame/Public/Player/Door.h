// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyFirstGameCharacter.h"
#include "Door.generated.h"

UCLASS()
class MYFIRSTGAME_API ADoor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoor();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
		class UBoxComponent* DoorCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door", meta = (AllowPrivateAccess = "true"))
		//TSubclassOf<class UStaticMeshComponent> DoorMesh;
		class UStaticMeshComponent* DoorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
		class UArrowComponent* DoorRoot;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//玩家进入门检测区域
	virtual void NotifyActorBeginOverlap(AActor* OtherActor)override;

	//玩家走出门检测区域
	virtual void NotifyActorEndOverlap(AActor* OtherActor)override;

private:
	AMyFirstGameCharacter* CurChar;

	//门是否可以被打开
	uint8 CanbOperate : 1;

	//门在开门转动状态
	uint8 IsInOpenRotate : 1;

	//门在关闭转动状态
	uint8 IsInCloseRotate : 1;

	//门在开启状态
	uint8 IsInOpen : 1;

	//
	uint8 IsInClose : 1;

	//当前门的角度
	float YawRotation = 0.f;
	
};
