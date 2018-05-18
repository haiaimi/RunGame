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

class UCurveFloat* RunGameHelper::CoinsArrangement = nullptr;
class UObjectLibrary* RunGameHelper::Library = nullptr;

void RunGameHelper::Initilize()
{
	/*����һ��ֱ�Ӽ���*/
	/*UCurveFloat* Temp = LoadObject<UCurveFloat>(nullptr, TEXT("/Game/Blueprint/CoinsArrange"));
	if (Temp)
		CoinsArrangement = Temp;*/

	/*������������UObject�⣬���Ǽ���һ���ļ��е���Դ���ڶ����Դ��ʱ��Ƚ�����*/
	Library = UObjectLibrary::CreateLibrary(UCurveFloat::StaticClass(), false, false);
	int32 AssetNums = Library->LoadAssetDataFromPath(TEXT("/Game/Blueprint/Curves"));   //����һ����Դ�ļ��У�һ��Ҫʹ�øü��غ��������򲻻���ص�������
	Library->LoadAssetsFromAssetData();       //���뵽�ڴ�
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
	if (!ensure(CoinsArrangement))return;   //δ������Դ��ֱ�ӷ���
	int32 CurveCoins = FMath::RandRange(0, 99);

	if (AttachedPlatform)
	{
		FMatrix Orient = FRotationMatrix(AttachedPlatform->GetActorRotation());    //ƽ̨����
		FVector SpawnDirX = Orient.GetUnitAxis(EAxis::X);
		FVector SpawnDirY = Orient.GetUnitAxis(EAxis::Y);
		FVector SpawnDirZ = Orient.GetUnitAxis(EAxis::Z);

		FVector FirstSpawnLoc;
		int32 ScorePosOnPlat = FMath::Rand() % 3;     //0Ϊ��1Ϊ�У�2Ϊ��

		//���������������λ�õ�Score
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
			//��ȡ��ǰӲ�ҵ���Ը߶�
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

			//SpawnBonus->SetOwner(ContextWorld->GetFirstPlayerController());    //�����ڼ��
			SpawnBonus->AttachToActor(AttachedPlatform, FAttachmentTransformRules::KeepWorldTransform);
			AttachedPlatform->OnDestory.AddUObject(SpawnBonus, &ABonus::DestroyActor);
			AttachedPlatform->OnFall.AddUObject(SpawnBonus, &ABonus::StartFall);
		}
	}
	//FDynamicMeshBuilder
	//SDPG_Foreground
}

void RunGameHelper::DrawMesh()
{
	//FDynamicMeshBuilder* MeshBuilder = new FDynamicMeshBuilder();

	//// ���㻺��
	//FDynamicMeshVertexBuffer* VertexBuffer = new FDynamicMeshVertexBuffer;
	//VertexBuffer->Vertices.SetNum(3);
	//VertexBuffer->Vertices[0] = FDynamicMeshVertex(FVector(0.f, 0.f, 0.f));
	//VertexBuffer->Vertices[1] = FDynamicMeshVertex(FVector(20000.f, 0.f, 0.f));
	//VertexBuffer->Vertices[2] = FDynamicMeshVertex(FVector(20000.f, 0.f, 11000.f));

	//// ��������
	//FDynamicMeshIndexBuffer* IndexBuffer = new FDynamicMeshIndexBuffer;
	//IndexBuffer->Indices.SetNum(3);
	//IndexBuffer->Indices[0] = 0;
	//IndexBuffer->Indices[1] = 1;
	//IndexBuffer->Indices[2] = 2;

	//MeshBuilder->AddVertex(FDynamicMeshVertex(FVector(0.f, 0.f, 0.f)));
	//MeshBuilder->AddVertex(FDynamicMeshVertex(FVector(20000.f, 0.f, 0.f)));
	//MeshBuilder->AddVertex(FDynamicMeshVertex(FVector(20000.f, 0.f, 11000.f)));

	//MeshBuilder->AddTriangle(0, 1, 2);

	//MeshBuilder->Draw()
}

void RunGameHelper::Clear()
{
	//Library->ClearLoaded();
	//CoinsArrangement = nullptr;
}