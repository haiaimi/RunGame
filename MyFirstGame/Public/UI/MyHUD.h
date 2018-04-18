// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MyHUD.generated.h"

/**
 * 
 */
UCLASS()
class MYFIRSTGAME_API AMyHUD : public AHUD
{
	GENERATED_BODY()
public:
		AMyHUD();
	
		virtual void DrawHUD()override;

		virtual void BeginPlay()override;

		virtual void Destroyed()override;
	
public:
		UTexture2D* CrosshairTex;

		uint32 bDrawCrosshair : 1;

		class UUserWidget* GameWidget;

		TSubclassOf<class UUserWidget> WidgetClass;

public: 
	void BuildHUD();
};
