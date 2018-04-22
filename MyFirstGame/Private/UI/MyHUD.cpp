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

const int PointNums = 50;  //用于绘制圆弧的顶点个数
const float ArcAngle = 60.f;    //60度圆弧

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
			 
			//UE_LOG(LogRunGame, Log, TEXT("已添加到玩家屏幕"))
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
		//绘制小地图
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

		    //绘制玩家点
			FCanvasTileItem PlayerPoint(FVector2D::ZeroVector, FVector2D::ZeroVector, FLinearColor::Red);
			PlayerPoint.Size = FVector2D(4.f, 4.f);
			Canvas->DrawItem(PlayerPoint, Canvas->ClipX - MapWidth / 2 - 2, Canvas->ClipY - MapHeight / 2 - 2);
			
			const FMatrix AdjustRotate = FRotationMatrix(FRotator(0.f, -90.f, 0.f));    //把三维空间的向量转换到二维屏幕坐标的矩阵
			const FMatrix MiniMapAdjust = FRotationMatrix(FRotator(0.f, -RGS->MiniMapCapture->RotatorYaw, 0.f));  //小地图方向适配
			const FVector CharacterDir = MPC->GetControlRotation().Vector().GetSafeNormal2D();      //获取玩家视角方向
			const FVector ToScreenVec = MiniMapAdjust.TransformVector(AdjustRotate.TransformVector(CharacterDir));

			//转换到屏幕二维空间的位置
			const FVector2D PlayerOnScreenPos = FVector2D(Canvas->ClipX - MapWidth / 2, Canvas->ClipY - MapHeight / 2);
			const FVector2D PlayerOnScreenVec = FVector2D(ToScreenVec.X, ToScreenVec.Y);

			//下面画线来实现玩家圆弧方向
			const float RelativeRadians = FMath::Acos(FVector2D::DotProduct(FVector2D(0.f, 1.f), PlayerOnScreenVec.GetSafeNormal()));   //玩家在小地图上的相对于屏幕原点的位置
			float RelativeDegrees = FMath::RadiansToDegrees(RelativeRadians);
			const float AngleStride = ArcAngle / PointNums;    

			if (FVector2D::DotProduct(FVector2D(1.f, 0.f), PlayerOnScreenVec.GetSafeNormal()) > 0)
				RelativeDegrees = -RelativeDegrees;

			UE_LOG(LogRunGame, Log, TEXT("相对角度: %f"), RelativeDegrees)
			
			for (int i = 0; i <= PointNums; ++i)
			{
				const float Degrees = 90.f - ArcAngle / 2 + i * AngleStride;
				const float Radians = FMath::DegreesToRadians(Degrees);
				float x = FMath::Cos(Radians) * 40;
				float y = FMath::Sin(Radians) * 40;
				FVector2D PointOnScreenToPlayer = FVector2D(x, y).GetRotated(RelativeDegrees);  
				PointOnScreenToPlayer = FVector2D(PointOnScreenToPlayer.X + PlayerOnScreenPos.X, PointOnScreenToPlayer.Y + PlayerOnScreenPos.Y);   
			
				Canvas->K2_DrawLine(PlayerOnScreenPos,       //起点
									PointOnScreenToPlayer,	//目标点
									1.f, FLinearColor::Blue);
			}
		}
	}
}


