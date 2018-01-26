// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define COLLISION_PROJECTILE   ECollisionChannel::ECC_GameTraceChannel1
#define COLLISION_BOOMQUERY    ECollisionChannel::ECC_GameTraceChannel2

namespace EWeaponType
{
	enum Type
	{
		Weapon_Instant,
		Weapon_Projectile,
		Weapon_Num
	};
}

UENUM()
namespace EPlatformDirection
{
	enum Type
	{
		Absolute_Left,
		Absolute_Forward,
		Absolute_Right
	};
}
