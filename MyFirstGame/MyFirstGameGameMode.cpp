// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "MyFirstGameGameMode.h"
#include "MyFirstGameCharacter.h"
#include "UI/MyHUD.h"
#include "UObject/ConstructorHelpers.h"
#include "RunGameState.h"
#include "Common/RunGameHelper.h"

AMyFirstGameGameMode::AMyFirstGameGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	PlayerControllerClass = AMyPlayerController::StaticClass();
	GameStateClass = ARunGameState::StaticClass();
	HUDClass = AMyHUD::StaticClass();
}

void AMyFirstGameGameMode::Destroyed()
{
	Super::Destroyed();
}
