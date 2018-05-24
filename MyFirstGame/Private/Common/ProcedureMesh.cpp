// Fill out your copyright notice in the Description page of Project Settings.

#include "ProcedureMesh.h"
#include "PrimitiveSceneProxy.h"
#include "SceneManagement.h"
#include "PrimitiveViewRelevance.h"
#include "RunGameHelper.h"
#include "Engine/Engine.h"
#include "DynamicMeshBuilder.h"
#include "Private/SceneRendering.h"
#include "ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/Material.h"
#include "PhysicsEngine/BodySetup.h"



UProcedureMesh::UProcedureMesh(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface>  MaterialFinder(TEXT("/Game/StarterContent/Materials/M_AssetPlatform"));
	ConstantMaterial = ObjectInitializer.CreateDefaultSubobject<UMaterialInstanceConstant>(this, TEXT("ConstantMaterial"));

	if (MaterialFinder.Succeeded())
	{
		ConstantMaterial->SetParentEditorOnly(MaterialFinder.Object);
		ConstantMaterial->PostEditChange();
	}

	ModelSetup = nullptr;
	bHasCustomNavigableGeometry = EHasCustomNavigableGeometry::Yes;
	CastShadow = true;
	bUseAsOccluder = true;
	bHiddenInGame = false;
}

FPrimitiveSceneProxy* UProcedureMesh::CreateSceneProxy()
{
	//可见UBoxComponent里的内容
	class FProceduralSceneProxy : public FPrimitiveSceneProxy
	{
	public:
		FProceduralSceneProxy(const UProcedureMesh* InComponent)
		:FPrimitiveSceneProxy(InComponent)
		{
			bWillEverBeLit = false;


			DynamicMeshBuilder = new FDynamicMeshBuilder(ERHIFeatureLevel::SM5);

			DynamicMeshBuilder->AddVertex(FVector(0.f, 0.f, 0.f), FVector2D(0.f, 0.f), FVector(0.f, 0.f, 1.f), FVector(0.f, 1.f, 0.f), FVector(0.f, 0.f, 1.f), FColor::Black);
			DynamicMeshBuilder->AddVertex(FVector(2000.f, 0.f, 0.f), FVector2D(0.f, 0.f), FVector(0.f, 0.f, 1.f), FVector(0.f, 1.f, 0.f), FVector(0.f, 0.f, 1.f), FColor::Black);
			DynamicMeshBuilder->AddVertex(FVector(2000.f, 0.f, 1100.f), FVector2D(0.f, 0.f), FVector(0.f, 0.f, 1.f), FVector(0.f, 1.f, 0.f), FVector(0.f, 0.f, 1.f), FColor::Black);

			DynamicMeshBuilder->AddTriangle(0, 1, 2);
			DynamicMeshBuilder->AddTriangle(2, 1, 0);
		}

		SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_ProceduralSceneProxy_GetDynamicMeshElements);
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
						//RunGameHelper::DrawMesh(pdi);
						/*FDynamicMeshBuilder DynamicMeshBuilder(ViewFamily.GetFeatureLevel());

						DynamicMeshBuilder.AddVertex(FVector(0.f, 0.f, 0.f), FVector2D(0.f, 0.f), FVector(0.f, 0.f, 1.f), FVector(0.f, 1.f, 0.f), FVector(0.f, 0.f, 1.f), FColor::Black);
						DynamicMeshBuilder.AddVertex(FVector(2000.f, 0.f, 0.f), FVector2D(0.f, 0.f), FVector(0.f, 0.f, 1.f), FVector(0.f, 1.f, 0.f), FVector(0.f, 0.f, 1.f), FColor::Black);
						DynamicMeshBuilder.AddVertex(FVector(2000.f, 0.f, 1100.f), FVector2D(0.f, 0.f), FVector(0.f, 0.f, 1.f), FVector(0.f, 1.f, 0.f), FVector(0.f, 0.f, 1.f), FColor::Black);

						DynamicMeshBuilder.AddTriangle(0, 1, 2);
						DynamicMeshBuilder.AddTriangle(2, 1, 0);*/
						////设置顶点缓冲
						////DynamicMeshBuilder.AddVertices(VertexBuffer);
						//////设置索引缓冲
						////DynamicMeshBuilder.AddTriangles(IndexBuffer);
						//TArray<FDynamicMeshVertex> VertexBuffer;

						//TArray<uint32> IndexBuffer;

						//VertexBuffer.SetNum(4);

						//VertexBuffer =
						//{
						//	FDynamicMeshVertex(FVector(0.f, 0.f, 0.f)),
						//	FDynamicMeshVertex(FVector(20000.f, 0.f, 0.f)),
						//	FDynamicMeshVertex(FVector(20000.f, 0.f, 11000.f)),
						//	FDynamicMeshVertex(FVector(20000.f, 20000.f, 0.f))
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

						////设置顶点缓冲
						//DynamicMeshBuilder.AddVertices(VertexBuffer);
						////设置索引缓冲 
						//DynamicMeshBuilder.AddTriangles(IndexBuffer);
						////UE_LOG(LogRunGame, Log, TEXT("找到材质资源"))
						FMatrix TranslationMatrix = FTranslationMatrix(FVector(0.f, 0.f, 1000.f));
						FMatrix ToLocalMatrix = FScaleMatrix(FVector(1.f, 1.f, 1.f)) * TranslationMatrix;

						DrawWireBox(pdi, FBox(FVector(0.f, 0.f, 0.f), FVector(100.f, 100.f, 100.f)), FColor::Black, 5, 0);
						//GEngine->
						DynamicMeshBuilder->Draw(pdi, ToLocalMatrix, UMaterial::GetDefaultMaterial(EMaterialDomain::MD_Surface)->GetRenderProxy(IsSelected()), SDPG_World);
						//pdi->View->Ele
						//DrawCylinder(pdi, FVector(0.f, 0.f, 0.f), FVector(0.f, 0.f, 1000.f), 500.f, 100, UMaterial::GetDefaultMaterial(MD_Surface)->GetRenderProxy(false), SDPG_Foreground);
						//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::Printf(TEXT("获取到PDI,VewsNum:%d"), Views.Num()));

						//pdi->DrawLine(FVector(0.f, 0.f, 0.f), FVector(100000.f, 10000.f, 10000.f), FLinearColor::Green, SDPG_World, 5.f);

						//if (View->bIsViewInfo)    //判断是否为ViewInfo类型
						//	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::Printf(TEXT("MeshElement的数目:%d,"), static_cast<const FViewInfo*>(View)->ViewMeshElements.Num()));
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
			Result.bRenderCustomDepth = ShouldRenderCustomDepth();
			Result.bRenderInMainPass = ShouldRenderInMainPass();
			Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
			Result.bOpaqueRelevance = true;

			//Result.bEditorPrimitiveRelevance = true;

			return Result;
		}

		~FProceduralSceneProxy()
		{
			delete DynamicMeshBuilder;
		}

		virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
		uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

		//const FMaterialRenderProxy* RenderProxy;

		FDynamicMeshBuilder* DynamicMeshBuilder;
	};

	return new FProceduralSceneProxy(this);
}

FBoxSphereBounds UProcedureMesh::CalcBounds(const FTransform& LocalToWorld) const
{
	return FBoxSphereBounds(FVector::ZeroVector, FVector(2000.f, 2000.f, 2000.f), 3000.f).TransformBy(LocalToWorld);
}

UBodySetup* UProcedureMesh::GetBodySetup()
{
	if (ModelSetup == nullptr)
	{
		// 不存在则创建一个BodySetup
		UBodySetup* NewBodySetup = NewObject<UBodySetup>(this, NAME_None, (IsTemplate() ? RF_Public : RF_NoFlags));
		NewBodySetup->BodySetupGuid = FGuid::NewGuid();

		NewBodySetup->bGenerateMirroredCollision = false;
		NewBodySetup->bDoubleSidedGeometry = true;
		NewBodySetup->CollisionTraceFlag =  CTF_UseComplexAsSimple ;
		ModelSetup = NewBodySetup;
	}

	return ModelSetup;
}
