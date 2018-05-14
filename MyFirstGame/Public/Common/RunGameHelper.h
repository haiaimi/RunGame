// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyFirstGame.h"

class ARunPlatform;

/**
 * ����������Դ���أ���һЩ��������
 */
class MYFIRSTGAME_API RunGameHelper
{
public:

	static void Initilize();

	static void ArrangeCoins(UWorld* ContextWorld, UClass* SpawnClass, ARunPlatform* const AttachedPlatform, TEnumAsByte<EPlatformDirection::Type> PreDir);

	static void Clear();

private:
	static class UCurveFloat* CoinsArrangement;

	static class UObjectLibrary* Library;
};
