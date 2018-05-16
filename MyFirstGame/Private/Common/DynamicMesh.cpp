// Fill out your copyright notice in the Description page of Project Settings.

#include "DynamicMesh.h"
#include "PrimitiveSceneProxy.h"
#include "SceneManagement.h"
#include "PrimitiveViewRelevance.h"


UDynamicMesh::UDynamicMesh(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PDI = nullptr;
}


FPrimitiveSceneProxy* UDynamicMesh::CreateSceneProxy()
{
	/** Represents a UBoxComponent to the scene manager. */
	class FDynamicSceneProxy : public FPrimitiveSceneProxy
	{
	public:
		FDynamicSceneProxy(const UDynamicMesh* InComponent)
		:FPrimitiveSceneProxy(InComponent)
		{
			bWillEverBeLit = true;
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

					FPrimitiveDrawInterface* pdi = Collector.GetPDI(ViewIndex);
					
					/*if (pdi != nullptr)
						PDI = pdi;*/
				}
			}
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			const bool bProxyVisible = true;

			const bool bShowForCollision = View->Family->EngineShowFlags.Collision && IsCollisionEnabled();

			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = (IsShown(View) && bProxyVisible) || bShowForCollision;
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			return Result;
		}
		virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
		uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }
	};

	return new FDynamicSceneProxy(this);
}

FBoxSphereBounds UDynamicMesh::CalcBounds(const FTransform& LocalToWorld) const
{
	return FBoxSphereBounds();
}
