// Fill out your copyright notice in the Description page of Project Settings.

#include "MyHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"

AMyHUD::AMyHUD()
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshaiTexObj(TEXT("/Game/ThirdPerson/Texture/FirstPersonCrosshair"));
	CrosshairTex = CrosshaiTexObj.Object;
	bDrawCrosshair = true;
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


