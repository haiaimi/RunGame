// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyFirstGame.h"

class ARunPlatform;
class FPrimitiveDrawInterface;

/**
 * ����������Դ���أ���һЩ��������
 */
class MYFIRSTGAME_API RunGameHelper
{
public:
	/*RunGameHelper();

	~RunGameHelper();*/

	static void Initilize();

	/**����һЩ��Դ�������ⲿ�࣬���ⱻ���*/
	static void LoadAsset(UObject* Outer);

	static void ArrangeCoins(UWorld* ContextWorld, UClass* SpawnClass, ARunPlatform* const AttachedPlatform, TEnumAsByte<EPlatformDirection::Type> PreDir);

	static void Clear();

	static void SetPDI(class FPrimitiveDrawInterface* PDIRef);

	static void DrawMesh(FPrimitiveDrawInterface* PDIRef);

private:
	static class UCurveFloat* CoinsArrangement;

	static class UObjectLibrary* Library;

	//static class FDynamicMeshBuilder* DynamicMeshBuilder;

	static class UMaterial* DynamicMeshMaterial;

	static TArray<FDynamicMeshVertex> VertexBuffer;

	static TArray<int32> IndexBuffer;
};
