// Fill out your copyright notice in the Description page of Project Settings.

#include "../../Public/Common/RunGameHelper.h"
#include "ConstructorHelpers.h"
#include "Curves/CurveFloat.h"
#include "RunPlatform.h"
#include "MyFirstGame.h"
#include "Bonus.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "UObjectGlobals.h"
#include "Engine/ObjectLibrary.h"

class UCurveFloat* RunGameHelper::CoinsArrangement = nullptr;
class UObjectLibrary* RunGameHelper::Library = nullptr;

void RunGameHelper::Initilize()
{
	/*方法一：直接加载*/
	/*UCurveFloat* Temp = LoadObject<UCurveFloat>(nullptr, TEXT("/Game/Blueprint/CoinsArrange"));
	if (Temp)
		CoinsArrangement = Temp;*/

	/*方法二：加载UObject库，这是加载一个文件夹的资源，在多个资源的时候比较适用*/
	Library = UObjectLibrary::CreateLibrary(UCurveFloat::StaticClass(), false, false);
	int32 AssetNums = Library->LoadAssetDataFromPath(TEXT("/Game/Blueprint/Curves"));   //加载一个资源文件夹，一定要使用该加载函数，否则不会加载到数组中
	Library->LoadAssetsFromAssetData();       //载入到内存
	//UE_LOG(LogRunGame, Log, TEXT("%d"), AssetNums)
		
	TArray<FAssetData> Assets;
	Library->GetAssetDataList(Assets);
	AssetNums = Assets.Num();
	for (int32 i = 0; i < AssetNums; ++i)
	{
		UE_LOG(LogRunGame, Log, TEXT("%s"), *(Assets[i].AssetName.ToString()))
		if (Assets[i].AssetName.IsEqual(TEXT("CoinsArrange")))
			CoinsArrangement = Cast<UCurveFloat>(Assets[i].GetAsset());
	}
}

void RunGameHelper::ArrangeCoins(UWorld* ContextWorld, UClass* SpawnClass, ARunPlatform* const AttachedPlatform, TEnumAsByte<EPlatformDirection::Type> PreDir)
{
	if (!ensure(CoinsArrangement))return;   //为创建资源，之间返回
	int32 CurveCoins = FMath::RandRange(0, 99);

	if (AttachedPlatform)
	{
		FMatrix Orient = FRotationMatrix(AttachedPlatform->GetActorRotation());    //平台方向
		FVector SpawnDirX = Orient.GetUnitAxis(EAxis::X);
		FVector SpawnDirY = Orient.GetUnitAxis(EAxis::Y);
		FVector SpawnDirZ = Orient.GetUnitAxis(EAxis::Z);

		FVector FirstSpawnLoc;
		int32 ScorePosOnPlat = FMath::Rand() % 3;     //0为左，1为中，2为右

		//下面就是设置三个位置的Score
		if (ScorePosOnPlat == 0)
			FirstSpawnLoc = AttachedPlatform->GetActorLocation() + SpawnDirY * (AttachedPlatform->GetPlatformWidth() - 20) + 30 * SpawnDirX + SpawnDirZ * 70;
		else if (ScorePosOnPlat == 1)
			FirstSpawnLoc = AttachedPlatform->GetActorLocation() + SpawnDirY * AttachedPlatform->GetPlatformWidth() / 2 + 30 * SpawnDirX + SpawnDirZ * 70;
		else if (ScorePosOnPlat == 2)
			FirstSpawnLoc = AttachedPlatform->GetActorLocation() + SpawnDirY * 20 + 30 * SpawnDirX + SpawnDirZ * 70;

		int32 BonusNum = 0;
		float XStride = 8.f;
		if (AttachedPlatform->PlatDir == PreDir)
			BonusNum = AttachedPlatform->GetPlatformLength() / 100.f;
		else
			BonusNum = AttachedPlatform->GetPlatformLength() / 100.f - 1;

		XStride /= (BonusNum - 1);
		for (int32 i = 0; i < BonusNum; i++)
		{
			float RelativeHeight = 0.f;
			//获取当前硬币的相对高度
			if (CurveCoins < 50)
			{
				RelativeHeight = 50.f*CoinsArrangement->GetFloatValue(i*XStride);
			}
			else
			{
				RelativeHeight = 0.f;
			}
			FTransform SpawnTrans = FTransform(AttachedPlatform->GetActorRotation(), FirstSpawnLoc + 100.f * i * SpawnDirX + RelativeHeight * SpawnDirZ);

			ABonus* SpawnBonus = ContextWorld->SpawnActorDeferred<ABonus>(SpawnClass, SpawnTrans, ContextWorld->GetFirstPlayerController());
			if (SpawnBonus != nullptr)
			{
				SpawnBonus->RotateStartTime = 0.1f*i;
				UGameplayStatics::FinishSpawningActor(SpawnBonus, SpawnTrans);
			}

			//SpawnBonus->SetOwner(ContextWorld->GetFirstPlayerController());    //会用于检测
			SpawnBonus->AttachToActor(AttachedPlatform, FAttachmentTransformRules::KeepWorldTransform);
			AttachedPlatform->OnDestory.AddUObject(SpawnBonus, &ABonus::DestroyActor);
			AttachedPlatform->OnFall.AddUObject(SpawnBonus, &ABonus::StartFall);
		}
	}
}

void RunGameHelper::Clear()
{
	//Library->ClearLoaded();
	//CoinsArrangement = nullptr;
}
