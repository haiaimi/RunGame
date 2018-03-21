// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "RunGameState.generated.h"

/**
 * 
 */
UCLASS()
class MYFIRSTGAME_API ARunGameState : public AGameStateBase
{
	GENERATED_UCLASS_BODY()
	
public:
	/**Íæ¼ÒµÃ·Ö*/
	float PlayerScore;
	
	float PlayerHeight;
};
