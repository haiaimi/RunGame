// Fill out your copyright notice in the Description page of Project Settings.

#include "MyHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"
#include "../UMG/Public/Blueprint/UserWidget.h"
#include "Player/MyPlayerController.h"
#include "RunGameState.h"
#include "RunMiniMapCapture.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture.h"

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
	DrawMiniMap();
}

void AMyHUD::BuildHUD()
{
	//尝试UMG画面显示在截屏中
	if (!GameWidget && WidgetClass)
	{
		AMyPlayerController* const MPC = Cast<AMyPlayerController>(GetOwningPlayerController());
		
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

void AMyHUD::DrawMiniMap()
{
	AMyPlayerController* const MPC = Cast<AMyPlayerController>(PlayerOwner);
	ARunGameState* const RGS = Cast<ARunGameState>(GetWorld()->GetGameState());

	if (RGS && RGS->MiniMapCapture != nullptr)
	{
		if (RGS->MiniMapCapture->GetCaptureComponent2D()->TextureTarget)
		{
			RGS->MiniMapCapture->GetCaptureComponent2D()->UpdateContent();
			float MapWidth = RGS->MiniMapCapture->MiniMapWidth;
			float MapHeight = RGS->MiniMapCapture->MiniMapHeight;

			FCanvasTileItem MinimapTileItem(FVector2D::ZeroVector, FVector2D::ZeroVector, FLinearColor::White);
			MinimapTileItem.Size = FVector2D(MapWidth, MapHeight);
			MinimapTileItem.Texture = RGS->MiniMapCapture->GetCaptureComponent2D()->TextureTarget->Resource;
			MinimapTileItem.BlendMode = ESimpleElementBlendMode::SE_BLEND_Opaque;       //非透明混合
			Canvas->DrawItem(MinimapTileItem, Canvas->ClipX - MapWidth, Canvas->ClipY - MapHeight);    //在hud中绘制地图
		}
	}
}


