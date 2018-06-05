// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "ProcedureMesh.generated.h"


struct FProcedureMeshVertex
{
	FProcedureMeshVertex() {}    //默认构造函数

	FProcedureMeshVertex(const FVector& InPosition) :
		Position(InPosition),
		TextureCoordinate(FVector2D(0.f, 0.f)),
		TangentX(FVector(1.f, 0.f, 0.f)),
		TangentZ(FVector(0.f, 0.f, 1.f)),
		Color(FColor::Black)
	{}

	FVector Position;
	FVector2D TextureCoordinate;
	FPackedNormal TangentX;
	FPackedNormal TangentZ;
	FColor Color;
};

/**
 * 
 */
UCLASS(hideCategories = (Object, LOD), meta = (BlueprintSpawnableComponent), ClassGroup = Rendering)
class MYFIRSTGAME_API UProcedureMesh : public UMeshComponent, public IInterface_CollisionDataProvider
{
	GENERATED_UCLASS_BODY()
	
public:
	/**初始化网格信息*/
	void InitMesh(const TArray<FProcedureMeshVertex>& InVertices, const TArray<uint32>& InIndices);

	/**更新碰撞体的构成信息*/
	void UpdateCollision();

public:
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual UBodySetup* GetBodySetup()override;

	virtual UMaterialInterface* GetMaterialFromCollisionFaceIndex(int32 FaceIndex, int32& SectionIndex) const override;

	// IInterface_CollisionDataProvider 接口内容
	virtual bool GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData)override;
	virtual bool ContainsPhysicsTriMeshData(bool InUseAllTriData) const override;
	virtual bool WantsNegXTriMesh()override { return false; };

private:
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	virtual int32 GetNumMaterials()const override;

public:

	class UMaterialInstanceConstant* ConstantMaterial;

	/**模型碰撞体相关*/
	UPROPERTY(Instanced)
	class UBodySetup* ModelSetup;

	TArray<FProcedureMeshVertex> Vertices;

	TArray<uint32> Indices;
};
