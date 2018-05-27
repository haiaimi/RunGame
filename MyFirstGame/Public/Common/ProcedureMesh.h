// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "ProcedureMesh.generated.h"

/**
 * 
 */
UCLASS()
class MYFIRSTGAME_API UProcedureMesh : public UMeshComponent
{
	GENERATED_UCLASS_BODY()
	
public:
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual UBodySetup* GetBodySetup()override;

	virtual UMaterialInterface* GetMaterialFromCollisionFaceIndex(int32 FaceIndex, int32& SectionIndex) const override;

private:
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	virtual int32 GetNumMaterials()const override;

public:

	class UMaterialInstanceConstant* ConstantMaterial;

	/**模型碰撞体相关*/
	UPROPERTY(Instanced)
	class UBodySetup* ModelSetup;
	
};
