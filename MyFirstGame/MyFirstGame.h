// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define COLLISION_PROJECTILE   ECollisionChannel::ECC_GameTraceChannel1
#define COLLISION_BOOMQUERY    ECollisionChannel::ECC_GameTraceChannel2

DECLARE_LOG_CATEGORY_EXTERN(LogRunGame, Log, All);

class FRunGameModule :public FDefaultGameModuleImpl
{
	virtual void StartupModule()override;

	virtual void ShutdownModule()override;
};

UENUM(BlueprintType)
namespace EWeaponType
{
	enum Type
	{
		Weapon_Instant,
		Weapon_Projectile,
		Weapon_Beam,
		Weapon_Num
	};
}

UENUM(BlueprintType)
namespace EPlatformDirection
{
	enum Type
	{
		Absolute_Left,
		Absolute_Forward,
		Absolute_Right
	};
}

UENUM(BlueprintType)
namespace EBonusType
{
	enum Type
	{
		Bonus_None,
		Bonus_Score,
		Bonus_Accelerate,
		Bonus_NoObstacle
	};
}

UENUM()
namespace EFlyObstacleToCharDir
{
	enum Type
	{
		FOTCD_Front,
		FOTCD_Right,
		FOTCD_Behind,
		FOTCD_Left
	};
}