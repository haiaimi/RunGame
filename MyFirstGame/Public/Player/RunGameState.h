// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "RunGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateScore, float, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRemind);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEMphasizeScore);

UCLASS()
class MYFIRSTGAME_API ARunGameState : public AGameStateBase
{
	GENERATED_UCLASS_BODY()
	
public:

	UPROPERTY(BlueprintAssignable)
	FUpdateScore UpdateScoreDelegate;

	UPROPERTY(BlueprintAssignable)
	FRemind RemindDelegate;

	/**��������Ŵ�*/
	UPROPERTY(BlueprintAssignable)
	FEMphasizeScore EmphasizeScoreDelegate;

	/**��ҵ÷�*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	float PlayerScore;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	float PlayerHeight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	float PlayerDistance;

	/**��ȡ������Ŀ*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	int32 Bonus_Score_Num;

public:

	void AddPlayerDistance(float AddDistance);

	void AddPlayerHeight(float AddHeight);

	void UpdatePlayerScore(float AddScore);

	void EndGame();

	void RestartGame();
};
