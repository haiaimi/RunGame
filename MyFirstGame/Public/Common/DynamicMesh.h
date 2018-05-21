// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "DynamicMesh.generated.h"

class FPrimitiveSceneProxy;
class FPrimitiveDrawInterface;
class UMaterialInstanceConstant;
/**
 * 该类用于获取 PDI接口进行动态绘图
 */
UCLASS()
class MYFIRSTGAME_API UDynamicMesh : public UPrimitiveComponent
{
	GENERATED_UCLASS_BODY()
	
public:
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

public:
	/**绘图接口*/
	class FPrimitiveDrawInterface* PDI;

	class UMaterialInstanceConstant* ConstantMaterial;
	
};

