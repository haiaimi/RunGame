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
#include "VertexFactory.h"
#include "RenderResource.h"

/**顶点缓冲类*/
class FProcedureVertexBuffer :public FVertexBuffer
{
public:
	virtual void InitRHI()override
	{
		FRHIResourceCreateInfo CreateInfo;
		VertexBufferRHI = RHICreateVertexBuffer(Vertices.Num() * sizeof(FDynamicMeshVertex), EBufferUsageFlags::BUF_Static, CreateInfo);    //创建顶点缓冲，并返回RHI接口

		//映射缓冲区的内存，并向其填充数据
		void* VertexBufferData = RHILockVertexBuffer(VertexBufferRHI, 0, Vertices.Num() * sizeof(FDynamicMeshVertex), RLM_WriteOnly);   
		FMemory::Memcpy(VertexBufferData, Vertices.GetData(), Vertices.Num() * sizeof(FDynamicMeshVertex));     //向缓冲区传入数据
		RHIUnlockVertexBuffer(VertexBufferRHI);    //关闭映射
	}

	virtual void ReleaseRHI()override
	{
		FVertexBuffer::ReleaseRHI();
	}

public:
	/**顶点数组*/
	TArray<FDynamicMeshVertex> Vertices;

};


/**索引缓冲类*/
class FProcedureIndexBuffer :public FIndexBuffer
{
public:
	virtual void InitRHI()override
	{
		// 操作同上，创建索引缓并填充数据
		FRHIResourceCreateInfo CreateInfo;
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(int32), Indices.Num() * sizeof(int32), BUF_Static, CreateInfo);

		void* Buffer = RHILockIndexBuffer(IndexBufferRHI, 0, Indices.Num() * sizeof(int32), RLM_WriteOnly);
		FMemory::Memcpy(Buffer, Indices.GetData(), Indices.Num() * sizeof(int32));
		RHIUnlockIndexBuffer(IndexBufferRHI);
	}


	virtual void ReleaseRHI()override
	{
		FIndexBuffer::ReleaseRHI();
	}

public:
	/**顶点索引数组*/
	TArray<int32> Indices;
};

/**输入布局，就是D3D中的InputLayout，其输入布局对应于FDynamicVertex中的内容*/
class FProcedureVertexFactory :public FLocalVertexFactory
{
public:
	FProcedureVertexFactory() :FLocalVertexFactory(ERHIFeatureLevel::SM5, "ProcedureVertexFactory") {}

public:
	// 输入布局初始化
	void Init(const FProcedureVertexBuffer* InVertexBuffer)
	{
		VertexBuffer = InVertexBuffer;

		FLocalVertexFactory* VertexFactory = this;
		const FProcedureVertexBuffer* ProcedureVertexBuffer = VertexBuffer;
		ENQUEUE_RENDER_COMMAND(InitProcedureVertexFactory)(
			[VertexFactory, ProcedureVertexBuffer](FRHICommandListImmediate& RHICmdList)
		{
			/**
			  先了解一下EVertexStreamUsage的作用，定义如下：

			  enum class EVertexStreamUsage : uint8
			  {
			  Default			= 0 << 0,        // 默认形式，如顶点
			  Instancing		= 1 << 0,        // 就如D3D中的硬件 Instance ，用于灵活变换DrawCall
			  Overridden		= 1 << 1,        // 覆盖
			  ManualFetch		= 1 << 2         // 手动获取，如取法线，纹理坐标，都需要在Shader中人工程序获取，但是如顶点就是直接通过顶点着色器，不是经过Shader人工程序获取
			  };

			*/
			FDataType NewData;
			//Position的输入
			NewData.PositionComponent = FVertexStreamComponent(
				ProcedureVertexBuffer,
				0,
				sizeof(FVector),
				VET_Float3,
				EVertexStreamUsage::Default
			);

			NewData.NumTexCoords = 1;       //默认只存在一个纹理坐标

			//TexCoord 纹理坐标输入
			NewData.TextureCoordinates.Add(FVertexStreamComponent(
				ProcedureVertexBuffer,
				0,
				sizeof(FVector2D),            //注意该Stride参数不是指单个元素的步程，而是指该组所有元素的步程，所以Stride = 输入类型的元素数目 * 该类型的宽度
				EVertexElementType::VET_Float2,
				EVertexStreamUsage::ManualFetch
			));

			// 两个Tangent输入分别为X，Y方向
			NewData.TangentBasisComponents[0] = FVertexStreamComponent(
				ProcedureVertexBuffer,
				0,
				2 * sizeof(FPackedNormal),
				EVertexElementType::VET_PackedNormal,
				EVertexStreamUsage::ManualFetch
			);

			NewData.TangentBasisComponents[1] = FVertexStreamComponent(
				ProcedureVertexBuffer,
				sizeof(FPackedNormal),
				2 * sizeof(FPackedNormal),        //由于这里连续存了两个Tangent元素，所以Stride为2倍FPackedNormal
				EVertexElementType::VET_PackedNormal,
				EVertexStreamUsage::ManualFetch
			);

			// Color颜色输入布局
			NewData.ColorComponent = FVertexStreamComponent(
				ProcedureVertexBuffer,
				0,
				sizeof(FColor),
				EVertexElementType::VET_Color,
				EVertexStreamUsage::ManualFetch
			);

			VertexFactory->SetData(NewData);
		}
		);
	}

private:
	const FProcedureVertexBuffer* VertexBuffer;
};

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
			VertexFactory.Init(&VertexBuffer);

			BeginInitResource(&VertexBuffer);
			BeginInitResource(&IndexBuffer);
			BeginInitResource(&VertexFactory);
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
						//DynamicMeshBuilder->Draw(pdi, ToLocalMatrix, UMaterial::GetDefaultMaterial(EMaterialDomain::MD_Surface)->GetRenderProxy(IsSelected()), SDPG_World);
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

		/**清空资源*/
		~FProceduralSceneProxy()
		{
			VertexBuffer.ReleaseResource();
			IndexBuffer.ReleaseResource();
			VertexFactory.ReleaseResource();
		}


		virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
		uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

		//const FMaterialRenderProxy* RenderProxy;

private:
		class FProcedureVertexBuffer VertexBuffer;
		class FProcedureIndexBuffer IndexBuffer;
		class FProcedureVertexFactory VertexFactory;
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
