// Fill out your copyright notice in the Description page of Project Settings.

#include "RunGameState.h"
#include "TimerManager.h"
#include "RunMiniMapCapture.h"
#include "MyFirstGameCharacter.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Weapon/Weapon_Gun.h"


ARunGameState::ARunGameState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;    //允许每帧更新

	PlayerDistance = 0.f;
	PlayerScore = 0.f;
	PlayerHeight = 0.f;
	Bonus_Score_Num = 0;
	HasSurvivedTime = -1.f;
	MiniMapCapture = nullptr;
	CharacterRef = nullptr;
}

void ARunGameState::BeginPlay()
{
	Super::BeginPlay();

	NotifyIntegerMinutes();
	HasSurvivedTime = 0.f;    //时间置零

	CharacterRef = Cast<AMyFirstGameCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
	if (!MiniMapCapture && CharacterRef)
	{
		FVector CaptureLocation = CharacterRef->GetActorLocation() + FVector(0.f, 0.f, 200.f);
		MiniMapCapture = GetWorld()->SpawnActor<ARunMiniMapCapture>(ARunMiniMapCapture::StaticClass(), CaptureLocation, CharacterRef->GetActorRotation());
		MiniMapCapture->GetCaptureComponent2D()->HiddenActors.Add(CharacterRef);
		MiniMapCapture->GetCaptureComponent2D()->HiddenActors.Add(CharacterRef->InterInventory[0]);
		MiniMapCapture->GetCaptureComponent2D()->HiddenActors.Add(CharacterRef->InterInventory[1]);
	}
}

void ARunGameState::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (MiniMapCapture && CharacterRef)
	{
		MiniMapCapture->SetSceneCaptureLocation(CharacterRef->GetActorLocation());
	}
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
	HasSurvivedTime = -1.f;
	GetWorldTimerManager().ClearTimer(GameTimer);
	GetWorldTimerManager().ValidateHandle(GameTimer);

	NotifyIntegerMinutes();
	HasSurvivedTime = 0.f;
}

void ARunGameState::NotifyIntegerMinutes()
{
	if (NotifyTimeDlegate.IsBound() && HasSurvivedTime != -1.f)
		NotifyTimeDlegate.Broadcast();

	HasSurvivedTime += 60.f;
	GetWorldTimerManager().SetTimer(GameTimer, this, &ARunGameState::NotifyIntegerMinutes, 60.f, false);
}

void ARunGameState::SetCaptureDir(EPlatformDirection::Type CurDir)
{
	if (MiniMapCapture)
	{
		if (MiniMapCapture->CachedDir != CurDir)
			MiniMapCapture->SetSceneCaptureDir(CurDir);
	}
}