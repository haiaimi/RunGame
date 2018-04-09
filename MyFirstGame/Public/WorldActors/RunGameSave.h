// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "RunGameSave.generated.h"

/**
 * 
 */
UCLASS()
class MYFIRSTGAME_API URunGameSave : public USaveGame
{
	GENERATED_UCLASS_BODY()
	
public:
	/**用于存储玩家分数*/
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<float> Scores;
	
	
};
