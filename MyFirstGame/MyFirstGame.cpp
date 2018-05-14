// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "MyFirstGame.h"
#include "Engine/EngineTypes.h"
#include "Modules/ModuleManager.h"
#include "Common/RunGameHelper.h"


DEFINE_LOG_CATEGORY(LogRunGame)

void FRunGameModule::StartupModule()
{
	RunGameHelper::Initilize();
}

void FRunGameModule::ShutdownModule()
{
	RunGameHelper::Clear();
}

IMPLEMENT_PRIMARY_GAME_MODULE(FRunGameModule, MyFirstGame, "MyFirstGame");
