// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyFirstGame.h"
#include "Bonus.generated.h"

USTRUCT()
struct FBonusData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Score")
	int32 BonusScore;

	UPROPERTY(EditDefaultsOnly, Category = "Speed")
	int32 BonusSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "NoObstacle")
	int32 NoObstacleTime;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TEnumAsByte<EBonusType::Type> BonusType;
	/**调整形状的大小*/
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	FVector ShapeScale;

	/**调整网格的定向*/
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	FRotator ShapeRotation;

	FBonusData()
	{
		BonusScore = 1;
		BonusSpeed = 50;
		NoObstacleTime = 5;
		BonusType = EBonusType::Bonus_None;
		ShapeRotation = FRotator::ZeroRotator;
		ShapeScale = FVector(1.f, 1.f, 1.f);
	}
};

UCLASS(Blueprintable)
class MYFIRSTGAME_API ABonus : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Bonus")
	class UStaticMeshComponent* BonusShape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus")
	class UBoxComponent* BonusQuery;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bonus")
	struct FBonusData BonusData;

	float RotateStartTime;

protected:
	// Called when the game starts or when spawned
	virtual void PostInitializeComponents()override;

public:
	// Called every frame
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)override;

public:
	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	FORCEINLINE int32 GetBonusScore() { return BonusData.BonusScore; }

	FORCEINLINE int32 GetBonusSpeed() { return BonusData.BonusSpeed; }

	void DestroyActor();

	void StartFall();
};