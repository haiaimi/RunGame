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
#include "DynamicMeshBuilder.h"
//#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

class UCurveFloat* RunGameHelper::CoinsArrangement = nullptr;
class UObjectLibrary* RunGameHelper::Library = nullptr;
class UMaterialInstanceDynamic* RunGameHelper::DynamicMeshMaterial = nullptr;
TArray<FDynamicMeshVertex> RunGameHelper::VertexBuffer = { FDynamicMeshVertex() };
TArray<uint32> RunGameHelper::IndexBuffer = { 0 };
bool RunGameHelper::bInitilized = false;
ERHIFeatureLevel::Type RunGameHelper::InFeatureLevel = ERHIFeatureLevel::Type::SM5;

//RunGameHelper::RunGameHelper()
//{
//	//DynamicMeshMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/StarterContent/Materials/M_AssetPlatform"));
//	//DynamicMeshBuilder = new class FDynamicMeshBuilder;
//
//	////TArray<FDynamicMeshVertex> VertexBuffer;
//	//VertexBuffer.SetNum(4);
//
//	//VertexBuffer =
//	//{
//	//	FDynamicMeshVertex(FVector(0.f, 0.f, 0.f)),
//	//	FDynamicMeshVertex(FVector(20000.f, 0.f, 0.f)),
//	//	FDynamicMeshVertex(FVector(20000.f, 0.f, 11000.f)),
//	//	FDynamicMeshVertex(FVector(20000.f, 20000.f, 0.f))
//	//};
//
//	//DynamicMeshBuilder->AddVertices(VertexBuffer);
//
//	////TArray<int32> IndexBuffer;
//	//IndexBuffer.SetNum(12);
//
//	//IndexBuffer =
//	//{
//	//	0,2,1,
//	//	1,2,3,
//	//	3,2,0,
//	//	0,1,3
//	//};
//
//	//DynamicMeshBuilder->AddTriangles(IndexBuffer);
//}
//
//RunGameHelper::~RunGameHelper()
//{
//	
//}

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


	//DynamicMeshBuilder = new class FDynamicMeshBuilder;
	//TArray<FDynamicMeshVertex> VertexBuffer;
	VertexBuffer.SetNum(4);

	VertexBuffer =
	{
		FDynamicMeshVertex(FVector(0.f, 0.f, 0.f)),
		FDynamicMeshVertex(FVector(20000.f, 0.f, 0.f)),
		FDynamicMeshVertex(FVector(20000.f, 0.f, 11000.f)),
		FDynamicMeshVertex(FVector(20000.f, 20000.f, 0.f))
	};

	//TArray<int32> IndexBuffer;
	IndexBuffer.SetNum(24);

	IndexBuffer =
	{
		0,2,1,
		1,2,0,
		1,2,3,
		3,2,1,
		3,2,0,
		0,2,3,
		0,1,3,
		3,1,0
	};

	bInitilized = true;
}

void RunGameHelper::LoadAsset(UObject* Outer)
{
	if (!ensure(Outer))return;
	UMaterialInterface* Material = LoadObject<UMaterialInterface>(Outer, TEXT("/Game/StarterContent/Materials/M_Brick_Clay_Beveled"));

	//DynamicMeshMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), Outer, TEXT("/Game/StarterContent/Materials/M_AssetPlatform")));
	
	//UMaterialInstanceDynamic* MaterialInstanceDynamic = NewObject<UMaterialInstanceDynamic>(Outer);
	if (Material)
	{
		DynamicMeshMaterial = UMaterialInstanceDynamic::Create(Material, Outer);
		/*MaterialInstance->SetParentEditorOnly(Material);
		MaterialInstance->PostEditChange();*/
		//DynamicMeshMaterial = MaterialInstance;
		UE_LOG(LogRunGame, Log, TEXT("材质：%s"), *DynamicMeshMaterial->GetMaterial()->GetName())
	}

	UWorld* CurWorld = Outer->GetWorld();     //当前所在世界
	if (CurWorld)
	{
		InFeatureLevel = CurWorld->Scene->GetFeatureLevel();

		UE_LOG(LogRunGame, Log, TEXT("FeatureLevel:%d"), (int32)InFeatureLevel)
	}
}

void RunGameHelper::ArrangeCoins(UWorld* ContextWorld, UClass* SpawnClass, ARunPlatform* const AttachedPlatform, TEnumAsByte<EPlatformDirection::Type> PreDir)
{
	if (!ensure(CoinsArrangement))return;   //未创建资源，直接返回
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
	//FDynamicMeshBuilder
	//SDPG_Foreground
}

void RunGameHelper::DrawMesh(FPrimitiveDrawInterface* PDIRef)
{
	//FDynamicMeshBuilder* MeshBuilder = new FDynamicMeshBuilder();

	//// 顶点缓冲
	//FDynamicMeshVertexBuffer* VertexBuffer = new FDynamicMeshVertexBuffer;
	//VertexBuffer->Vertices.SetNum(3);
	//VertexBuffer->Vertices[0] = FDynamicMeshVertex(FVector(0.f, 0.f, 0.f));
	//VertexBuffer->Vertices[1] = FDynamicMeshVertex(FVector(20000.f, 0.f, 0.f));
	//VertexBuffer->Vertices[2] = FDynamicMeshVertex(FVector(20000.f, 0.f, 11000.f));

	//// 索引缓冲
	//FDynamicMeshIndexBuffer* IndexBuffer = new FDynamicMeshIndexBuffer;
	//IndexBuffer->Indices.SetNum(3);
	//IndexBuffer->Indices[0] = 0;
	//IndexBuffer->Indices[1] = 1;
	//IndexBuffer->Indices[2] = 2;

	//VertexBuffer.SetNum(4);

	//VertexBuffer =
	//{
	//	FDynamicMeshVertex(FVector(0.f, 0.f, 0.f)),
	//	FDynamicMeshVertex(FVector(2000.f, 0.f, 0.f)),
	//	FDynamicMeshVertex(FVector(2000.f, 0.f, 1100.f)),
	//	FDynamicMeshVertex(FVector(2000.f, 2000.f, 0.f))
	//};

	////TArray<int32> IndexBuffer;
	//IndexBuffer.SetNum(24);

	//IndexBuffer =
	//{
	//	0,2,1,
	//	1,2,0,
	//	1,2,3,
	//	3,2,1,
	//	3,2,0,
	//	0,2,3,
	//	0,1,3,
	//	3,1,0
	//};

	if (bInitilized/* && DynamicMeshMaterial != nullptr*/)
	{
		FDynamicMeshBuilder DynamicMeshBuilder(InFeatureLevel);

		/*DynamicMeshBuilder.AddVertex(FVector(0.f, 0.f, 0.f), FVector2D(0.f, 0.f), FVector(0.f, 0.f, 1.f), FVector(0.f, 1.f, 0.f), FVector(0.f, 0.f, 1.f), FColor::Black);
		DynamicMeshBuilder.AddVertex(FVector(2000.f, 0.f, 0.f), FVector2D(0.f, 0.f), FVector(0.f, 0.f, 1.f), FVector(0.f, 1.f, 0.f), FVector(0.f, 0.f, 1.f), FColor::Black);
		DynamicMeshBuilder.AddVertex(FVector(2000.f, 0.f, 1100.f), FVector2D(0.f, 0.f), FVector(0.f, 0.f, 1.f), FVector(0.f, 1.f, 0.f), FVector(0.f, 0.f, 1.f), FColor::Black);
		
		DynamicMeshBuilder.AddTriangle(0, 1, 2);
		DynamicMeshBuilder.AddTriangle(2, 1, 0);*/
		//设置顶点缓冲
		DynamicMeshBuilder.AddVertices(VertexBuffer);
		//设置索引缓冲 
		DynamicMeshBuilder.AddTriangles(IndexBuffer);
		
		FMatrix TranslationMatrix = FTranslationMatrix(FVector(0.f, 0.f, 1000.f));
		FMatrix ToLocalMatrix = FScaleMatrix(FVector(1.f, 1.f, 1.f)) * TranslationMatrix;
		DynamicMeshBuilder.Draw(PDIRef, ToLocalMatrix, UMaterial::GetDefaultMaterial(MD_Surface)->GetRenderProxy(false), 0);
	}
}

void RunGameHelper::ScreenMessageDebug(FString&& InString,FColor FontColor)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FontColor, InString);
	}
}

void RunGameHelper::LogDebug(FString InString)
{
	UE_LOG(LogRunGame, Log, TEXT("%s"), *InString)
}

void RunGameHelper::SetPDI(FPrimitiveDrawInterface* PDIRef)
{

}

void RunGameHelper::Clear()
{
	//Library->ClearLoaded();
	//CoinsArrangement = nullptr;
	VertexBuffer.Empty();
	IndexBuffer.Empty();
}


