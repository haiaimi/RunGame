// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Weapon_Gun.h"
#include "Weapon_Gun_Beam.generated.h"

/**
 * 
 */
UCLASS()
class MYFIRSTGAME_API AWeapon_Gun_Beam : public AWeapon_Gun
{
	GENERATED_UCLASS_BODY()
	
public:
	
	void Fire(FVector FireDir)override;

public:
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction);
};
