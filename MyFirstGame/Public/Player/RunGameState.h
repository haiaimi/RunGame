// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MyFirstGame.h"
#include "RunGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateScore, float, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRemind);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEMphasizeScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNotifyTime);

UCLASS()
class MYFIRSTGAME_API ARunGameState : public AGameStateBase
{
	GENERATED_UCLASS_BODY()
	
public:

	UPROPERTY(BlueprintAssignable)
	FUpdateScore UpdateScoreDelegate;

	UPROPERTY(BlueprintAssignable)
	FRemind RemindDelegate;

	/**用于字体放大*/
	UPROPERTY(BlueprintAssignable)
	FEMphasizeScore EmphasizeScoreDelegate;

	UPROPERTY(BlueprintAssignable)
	FNotifyTime NotifyTimeDlegate;

	/**玩家得分*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	float PlayerScore;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	float PlayerHeight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	float PlayerDistance;

	/**获取奖励数目*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	int32 Bonus_Score_Num;

	/**用于玩家存活时间计时*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	FTimerHandle GameTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	float HasSurvivedTime;

	/**小地图捕捉*/
	class ARunMiniMapCapture* MiniMapCapture;

	class AMyFirstGameCharacter* CharacterRef;

public:
	virtual void BeginPlay()override;

	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)override;

public:

	void AddPlayerDistance(float AddDistance);

	void AddPlayerHeight(float AddHeight);

	void UpdatePlayerScore(float AddScore);

	void EndGame();

	void RestartGame();

	/**整数时间时进行通知*/
	void NotifyIntegerMinutes();

	void SetCaptureDir(EPlatformDirection::Type CurDir);
};
