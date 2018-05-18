// Fill out your copyright notice in the Description page of Project Settings.

#include "DynamicMesh.h"
#include "PrimitiveSceneProxy.h"
#include "SceneManagement.h"
#include "PrimitiveViewRelevance.h"
#include "RunGameHelper.h"
#include "Engine/Engine.h"
#include "DynamicMeshBuilder.h"


UDynamicMesh::UDynamicMesh(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PDI = nullptr;
}

FPrimitiveSceneProxy* UDynamicMesh::CreateSceneProxy()
{ 
	//可见UBoxComponent里的内容
	class FDynamicSceneProxy : public FPrimitiveSceneProxy
	{
	public:
		FDynamicSceneProxy(const UDynamicMesh* InComponent)
		:FPrimitiveSceneProxy(InComponent)
		{
			bWillEverBeLit = false;
			//HelperInstance = new RunGameHelper;
		}

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_DynamicSceneProxy_GetDynamicMeshElements);
			const FMatrix& LocalToWorld = GetLocalToWorld();

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];

					//const FLinearColor DrawColor = GetViewSelectionColor(BoxColor, *View, IsSelected(), IsHovered(), false, IsIndividuallySelected());

					//获取PDI
					FPrimitiveDrawInterface* pdi = Collector.GetPDI(ViewIndex);
					
					if (pdi != nullptr)
					{
						RunGameHelper::DrawMesh(pdi);
						GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::Printf(TEXT("获取到PDI,VewsNum:%d"), Views.Num()));
						pdi->DrawLine(FVector(0.f, 0.f, 0.f), FVector(100000.f, 10000.f, 10000.f), FLinearColor::Green, SDPG_World, 5.f);
					}
				}
			}
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = IsShown(View);
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = true;
			//Result.bEditorPrimitiveRelevance = true;
		
			return Result;
		}
		virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
		uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

		~FDynamicSceneProxy()
		{
			FPrimitiveSceneProxy::~FPrimitiveSceneProxy();
		}
	};


	return new FDynamicSceneProxy(this);
}

FBoxSphereBounds UDynamicMesh::CalcBounds(const FTransform& LocalToWorld) const
{
	return FBoxSphereBounds(FVector::ZeroVector, FVector(2000.f,2000.f, 2000.f), 3000.f).TransformBy(LocalToWorld);
}
