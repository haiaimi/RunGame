// Fill out your copyright notice in the Description page of Project Settings.

#include "RunGameState.h"


ARunGameState::ARunGameState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PlayerDistance = 0.f;
	PlayerScore = 0.f;
	PlayerHeight = 0.f;
	Bonus_Score_Num = 0;
}


void ARunGameState::AddPlayerDistance(float AddDistance)
{
	PlayerDistance += AddDistance / 100.f;     //单位转换成米
	UpdatePlayerScore(AddDistance * 10.f / 100.f);     
}

void ARunGameState::AddPlayerHeight(float NewHeight)
{
	float AddHeight = NewHeight / 100.f - PlayerHeight;
	PlayerHeight = NewHeight / 100.f;
	UpdatePlayerScore(AddHeight * 50.f / 100.f);    
}

void ARunGameState::UpdatePlayerScore(float AddScore)
{
	PlayerScore += AddScore;

	if (UpdateScoreDelegate.IsBound())
	{
		UpdateScoreDelegate.Broadcast(PlayerScore);
	}
}

void ARunGameState::EndGame()
{

}

void ARunGameState::RestartGame()
{
	PlayerScore = 0.f;
	PlayerHeight = 0.f;
	PlayerDistance = 0.f;
}
