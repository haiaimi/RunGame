// Fill out your copyright notice in the Description page of Project Settings.

#include "MyHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"
#include "../UMG/Public/Blueprint/UserWidget.h"
#include "Player/MyPlayerController.h"

AMyHUD::AMyHUD()
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshaiTexObj(TEXT("/Game/ThirdPerson/Texture/FirstPersonCrosshair"));
	static ConstructorHelpers::FClassFinder<UUserWidget> GameWidgetClass(TEXT("/Game/UI/RunHUD"));
	
	if (GameWidgetClass.Succeeded())
		WidgetClass = GameWidgetClass.Class;
	else
		WidgetClass = nullptr;

	CrosshairTex = CrosshaiTexObj.Object;
	GameWidget = nullptr;
	bDrawCrosshair = true;
}

void AMyHUD::BeginPlay()
{
	Super::BeginPlay();

	BuildHUD();
}

void AMyHUD::Destroyed()
{
	Super::Destroyed();
}

void AMyHUD::DrawHUD()
{
	Super::DrawHUD();

	if (bDrawCrosshair)
	{
		const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

		const FVector2D CrosshairPosition(Center.X - (CrosshairTex->GetSurfaceWidth()*0.5f),
			Center.Y - (CrosshairTex->GetSurfaceHeight()*0.5f));

		FCanvasTileItem TileItem(CrosshairPosition, CrosshairTex->Resource, FLinearColor::White);
		TileItem.BlendMode = SE_BLEND_Translucent;

		Canvas->DrawItem(TileItem);
	}
}

void AMyHUD::BuildHUD()
{
	//尝试UMG画面显示在截屏中
	if (!GameWidget && WidgetClass)
	{
		AMyPlayerController* MPC = Cast<AMyPlayerController>(GetOwningPlayerController());
		
		GameWidget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
		if (MPC->GetLocalPlayer())
		{
			GameWidget->SetOwningLocalPlayer(MPC->GetLocalPlayer());
			GameWidget->AddToPlayerScreen(1);
			 
			UE_LOG(LogRunGame, Log, TEXT("已添加到玩家屏幕"))
		}
		else
			GameWidget->AddToViewport();
	}
}


