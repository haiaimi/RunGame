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
#include "Stats.h"

DECLARE_STATS_GROUP(TEXT("MyProceduralMesh"), STATGROUP_MProceduralMesh, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("Create ProcMesh Proxy"), STAT_ProceduralMesh_CreateSceneProxy, STATGROUP_MProceduralMesh);
DECLARE_CYCLE_STAT(TEXT("Get DynamicProcMesh Elements"), STAT_ProceduralMesh_GetDynamicMeshElements, STATGROUP_MProceduralMesh);


/**顶点缓冲类*/
class FProcedureVertexBuffer :public FRenderResource
{
public:
	virtual void InitRHI()override
	{
		FRHIResourceCreateInfo CreateInfo_1, CreateInfo_2, CreateInfo_3, CreateInfo_4;
		PositionBuffer.VertexBufferRHI = RHICreateVertexBuffer(Vertices.Num() * sizeof(FVector), EBufferUsageFlags::BUF_Static | EBufferUsageFlags::BUF_ShaderResource, CreateInfo_1);    //创建顶点缓冲，并返回VertexBufferRHI，其实就是指向缓冲所在内存的指针
		TangentBuffer.VertexBufferRHI = RHICreateVertexBuffer(2 * Vertices.Num() * sizeof(FPackedNormal), EBufferUsageFlags::BUF_Static | EBufferUsageFlags::BUF_ShaderResource, CreateInfo_2);   //由于要创建SRV所以要指定缓冲区类型
		TexCoordBuffer.VertexBufferRHI = RHICreateVertexBuffer(Vertices.Num() * sizeof(FVector2D), EBufferUsageFlags::BUF_Static | EBufferUsageFlags::BUF_ShaderResource, CreateInfo_3);
		ColorBuffer.VertexBufferRHI = RHICreateVertexBuffer(Vertices.Num() * sizeof(FColor), EBufferUsageFlags::BUF_Static | EBufferUsageFlags::BUF_ShaderResource, CreateInfo_4);

		//创建ShaderResourceView，着色器资源
		TangentBufferSRV = RHICreateShaderResourceView(TangentBuffer.VertexBufferRHI, 4, PF_R8G8B8A8);
		TexCoordBufferSRV = RHICreateShaderResourceView(TexCoordBuffer.VertexBufferRHI, 8, PF_G32R32F);
		ColorBufferSRV = RHICreateShaderResourceView(ColorBuffer.VertexBufferRHI, 4, PF_R8G8B8A8);

		PositionBufferSRV = RHICreateShaderResourceView(PositionBuffer.VertexBufferRHI, 4, PF_R32_FLOAT);    //用于深度检测

		//映射缓冲区的内存，4个缓冲区
		FVector* PositionBufferData = static_cast<FVector*>(RHILockVertexBuffer(PositionBuffer.VertexBufferRHI, 0, Vertices.Num() * sizeof(FVector), RLM_WriteOnly));
		FPackedNormal* TangentBufferData = static_cast<FPackedNormal*>(RHILockVertexBuffer(TangentBuffer.VertexBufferRHI, 0, 2 * Vertices.Num() * sizeof(FPackedNormal), RLM_WriteOnly));
		FVector2D* TexCoordBufferData = static_cast<FVector2D*>(RHILockVertexBuffer(TexCoordBuffer.VertexBufferRHI, 0, Vertices.Num() * sizeof(FVector2D), RLM_WriteOnly));
		FColor* ColorBufferData = static_cast<FColor*>(RHILockVertexBuffer(ColorBuffer.VertexBufferRHI, 0, Vertices.Num() * sizeof(FColor), RLM_WriteOnly));

		//填充数据
		for (int i = 0; i < Vertices.Num(); i++)
		{
			PositionBufferData[i] = Vertices[i].Position;      //顶点数据
			TangentBufferData[2 * i] = Vertices[i].TangentX;    //切线数据
			TangentBufferData[2 * i + 1] = Vertices[i].TangentZ;
			TexCoordBufferData[i] = Vertices[i].TextureCoordinate;   //UV数据
			ColorBufferData[i] = Vertices[i].Color;          //颜色数据
		}
		
		//关闭映射
		RHIUnlockVertexBuffer(PositionBuffer.VertexBufferRHI);  
		RHIUnlockVertexBuffer(TangentBuffer.VertexBufferRHI);
		RHIUnlockVertexBuffer(TexCoordBuffer.VertexBufferRHI);
		RHIUnlockVertexBuffer(ColorBuffer.VertexBufferRHI);
	}

	void InitResource() override
	{
		// 初始化数据
		FRenderResource::InitResource();
		PositionBuffer.InitResource();
		TangentBuffer.InitResource();
		TexCoordBuffer.InitResource();
		ColorBuffer.InitResource();

		UE_LOG(LogRunGame, Log, TEXT("Vertex Init"))
	}

	void ReleaseResource() override
	{
		//释放资源
		FRenderResource::ReleaseResource();
		TangentBufferSRV.SafeRelease();
		TexCoordBufferSRV.SafeRelease();
		ColorBufferSRV.SafeRelease();
		PositionBufferSRV.SafeRelease();

		PositionBuffer.ReleaseResource();
		TangentBuffer.ReleaseResource();
		TexCoordBuffer.ReleaseResource();
		ColorBuffer.ReleaseResource();

		UE_LOG(LogRunGame, Log, TEXT("Vertex Release"))
	}

	virtual void ReleaseRHI()override
	{
		FRenderResource::ReleaseRHI();
	}

public:
	/**顶点数组*/
	TArray<FProcedureMeshVertex> Vertices;

	FVertexBuffer PositionBuffer;
	FVertexBuffer TangentBuffer;
	FVertexBuffer TexCoordBuffer;
	FVertexBuffer ColorBuffer;

	FShaderResourceViewRHIRef TangentBufferSRV;
	FShaderResourceViewRHIRef TexCoordBufferSRV;
	FShaderResourceViewRHIRef ColorBufferSRV;
	FShaderResourceViewRHIRef PositionBufferSRV;
};


/**索引缓冲类*/
class FProcedureIndexBuffer :public FIndexBuffer
{
public:
	virtual void InitRHI()override
	{
		// 操作同上，创建索引缓并填充数据
		FRHIResourceCreateInfo CreateInfo;
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint32), Indices.Num() * sizeof(uint32), BUF_Static, CreateInfo);      //创建索引缓冲

		void* Buffer = RHILockIndexBuffer(IndexBufferRHI, 0, Indices.Num() * sizeof(uint32), RLM_WriteOnly); 
		FMemory::Memcpy(Buffer, Indices.GetData(), Indices.Num() * sizeof(uint32));
		RHIUnlockIndexBuffer(IndexBufferRHI);

		UE_LOG(LogRunGame, Log, TEXT("Index Init"))
	}


	virtual void ReleaseRHI()override
	{
		FIndexBuffer::ReleaseRHI();
		UE_LOG(LogRunGame, Log, TEXT("Factory Release"))
	}

public:
	/**顶点索引数组*/
	TArray<uint32> Indices;
};

/**输入布局，就是D3D中的InputLayout，其输入布局对应于FProcedureMeshVertex中的内容*/
class FProcedureVertexFactory :public FLocalVertexFactory
{
public:
	FProcedureVertexFactory() :FLocalVertexFactory(ERHIFeatureLevel::SM5, "FProcedureVertexFactory") {}

	FProcedureVertexFactory(ERHIFeatureLevel::Type InFeatureLevel, const FProcedureVertexBuffer* InVertexBuffer) :FLocalVertexFactory(InFeatureLevel, "FProcedureVertexFactory"),VertexBuffer(InVertexBuffer) {}

public:
	// 输入布局初始化
	void InitResource()override
	{
		/*if (IsInRenderingThread())
		{
			FDataType TheData;
			TheData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FProcedureMeshVertex, Position, VET_Float3);
			TheData.TextureCoordinates.Add(
				FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FProcedureMeshVertex, TextureCoordinate), sizeof(FProcedureMeshVertex), VET_Float2)
			);
			TheData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FProcedureMeshVertex, TangentX, VET_PackedNormal);
			TheData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FProcedureMeshVertex, TangentZ, VET_PackedNormal);
			TheData.ColorComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FProcedureMeshVertex, Color, VET_Color);
			SetData(TheData);
		}*/

		FProcedureVertexFactory* VertexFactory = this;
		const FProcedureVertexBuffer* ProcedureVertexBuffer = VertexBuffer;
		ENQUEUE_RENDER_COMMAND(InitProcedureVertexFactory)(
			[VertexFactory,ProcedureVertexBuffer](FRHICommandList& RHICmdList)
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
					&ProcedureVertexBuffer->PositionBuffer,
					0,
					sizeof(FVector),
					VET_Float3,
					EVertexStreamUsage::Default
				);

				NewData.NumTexCoords = 1;       //默认只存在一个纹理坐标
				NewData.LightMapCoordinateIndex = 0;
				NewData.TangentsSRV = ProcedureVertexBuffer->TangentBufferSRV;
				NewData.TextureCoordinatesSRV = ProcedureVertexBuffer->TexCoordBufferSRV;
				NewData.ColorComponentsSRV = ProcedureVertexBuffer->ColorBufferSRV;
				NewData.PositionComponentSRV = ProcedureVertexBuffer->PositionBufferSRV;

												//TexCoord 纹理坐标输入
				NewData.TextureCoordinates.Add(FVertexStreamComponent(
					&ProcedureVertexBuffer->TexCoordBuffer,
					0,
					sizeof(FVector2D),                                
					EVertexElementType::VET_Float2,
					EVertexStreamUsage::ManualFetch
				));

				// 两个Tangent输入分别为X，Y方向
				NewData.TangentBasisComponents[0] = FVertexStreamComponent(
					&ProcedureVertexBuffer->TangentBuffer,
					0,
					2 * sizeof(FPackedNormal),
					EVertexElementType::VET_PackedNormal,
					EVertexStreamUsage::ManualFetch
				);

				NewData.TangentBasisComponents[1] = FVertexStreamComponent(
					&ProcedureVertexBuffer->TangentBuffer,
					sizeof(FPackedNormal),              //由于这里连续存了两个Tangent元素，所以Stride为2倍FPackedNormal
					2 * sizeof(FPackedNormal),
					EVertexElementType::VET_PackedNormal,
					EVertexStreamUsage::ManualFetch
				);

				// Color颜色输入布局
				NewData.ColorComponent = FVertexStreamComponent(
					&ProcedureVertexBuffer->ColorBuffer,
					0,                                //该宏用于计算结构体内的成员偏移量（字节为单位）
					sizeof(FColor),
					EVertexElementType::VET_Color,
					EVertexStreamUsage::ManualFetch
				);

				VertexFactory->SetData(NewData);
			}
		);

		FLocalVertexFactory::InitResource(); //该函数只在渲染线程中执行
		UE_LOG(LogRunGame, Log, TEXT("Factory Init"))
	}

private:
	const FProcedureVertexBuffer* VertexBuffer;
};

UProcedureMesh::UProcedureMesh(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface>  MaterialFinder(TEXT("/Game/StarterContent/Materials/M_AssetPlatform"));

	ModelSetup = nullptr;
	CastShadow = true;
	bHiddenInGame = false;
	SetCollisionResponseToAllChannels(ECR_Block);   //测试碰撞，阻挡一切物体
	//SetCollisionProfileName(TEXT("BlockAll"));
}

void UProcedureMesh::InitMesh(const TArray<FProcedureMeshVertex>& InVertices, const TArray<uint32>& InIndices)
{
	Vertices = Vertices;
	Indices = InIndices;
}

FPrimitiveSceneProxy* UProcedureMesh::CreateSceneProxy()
{
	SCOPE_CYCLE_COUNTER(STAT_ProceduralMesh_CreateSceneProxy);
	//可见UBoxComponent里的内容
	class FProceduralSceneProxy final : public FPrimitiveSceneProxy
	{
	public:
		FProceduralSceneProxy(const UProcedureMesh* InComponent)
		:FPrimitiveSceneProxy(InComponent),
		 MaterialRelevance(InComponent->GetMaterialRelevance(GetScene().GetFeatureLevel()))
		{
			VertexBuffer = nullptr;
			IndexBuffer = nullptr;
			VertexFactory = nullptr;
			VertexBuffer = new FProcedureVertexBuffer;
			VertexBuffer->Vertices.SetNum(4);      //此步多余，会引起崩溃
			MeshBody = InComponent->ModelSetup;

			VertexBuffer->Vertices =
			{
				FProcedureMeshVertex(FVector(0.f, 0.f, 0.f)),
				FProcedureMeshVertex(FVector(200.f, 0.f, 0.f)),
				FProcedureMeshVertex(FVector(200.f, 0.f, 100.f)),
				FProcedureMeshVertex(FVector(200.f, 200.f, 0.f))
			};

			//TArray<int32> IndexBuffer;
			IndexBuffer = new FProcedureIndexBuffer;
			//IndexBuffer->Indices.SetNum(24);

			IndexBuffer->Indices =
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

			TArray<FVector> CollisionVertex;
			for (int32 i = 0; i < VertexBuffer->Vertices.Num(); ++i)
			{
				CollisionVertex.Add(VertexBuffer->Vertices[i].Position);
			}
			if (MeshBody)
				MeshBody->UpdateTriMeshVertices(CollisionVertex);

			// 下面是初始化资源，有顶点/索引缓冲/输入布局
			BeginInitResource(VertexBuffer);
			BeginInitResource(IndexBuffer);

			VertexFactory = new FProcedureVertexFactory(InComponent->GetWorld()->Scene->GetFeatureLevel(), VertexBuffer);
			BeginInitResource(VertexFactory);
			MaterialProxy = UMaterial::GetDefaultMaterial(EMaterialDomain::MD_Surface)->GetRenderProxy(IsSelected());

			UE_LOG(LogRunGame, Log, TEXT("Proxy Construction"))
		}

		/**清空资源*/
		virtual ~FProceduralSceneProxy()
		{
			if (VertexBuffer)
			{
				VertexBuffer->ReleaseResource();
				delete VertexBuffer;
			}
			if (IndexBuffer)
			{
				IndexBuffer->ReleaseResource();
				delete IndexBuffer;
			}
			if (VertexFactory)
			{
				VertexFactory->ReleaseResource();
				delete VertexFactory;
			}

			UE_LOG(LogRunGame, Log, TEXT("Proxy Destruction"))
		}

		SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
		{
			SCOPE_CYCLE_COUNTER(STAT_ProceduralMesh_GetDynamicMeshElements);
			const FMatrix& LocalToWorld = GetLocalToWorld();

			//auto WireFrameMaterialInstance = new FColoredMaterialRenderProxy(GEngine->WireframeMaterial->GetRenderProxy(IsSelected()), FLinearColor::Black);
			//Collector.RegisterOneFrameMaterialProxy(WireFrameMaterialInstance);       //创建网格材质，需要的时候使用

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];

					FMeshBatch& Mesh = Collector.AllocateMesh();
					//FMeshBatch Mesh;
					FMeshBatchElement& BatchElement = Mesh.Elements[0];
					BatchElement.IndexBuffer = IndexBuffer;
					Mesh.bWireframe = false;   //不是网格型
					Mesh.VertexFactory = VertexFactory;      //输入布局
					Mesh.MaterialRenderProxy = MaterialProxy;
					BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, UseEditorDepthTest());    //设置全局缓冲区参数
					BatchElement.FirstIndex = 0;       //
					BatchElement.NumPrimitives = IndexBuffer->Indices.Num() / 3;    //绘制的图元数目
					BatchElement.MinVertexIndex = 0;
					BatchElement.MaxVertexIndex = VertexBuffer->Vertices.Num() - 1;
					Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
					Mesh.Type = EPrimitiveType::PT_TriangleList;     //基本图元为三角形
					Mesh.DepthPriorityGroup = SDPG_World;      //深度检测优先顺序
					Mesh.bCanApplyViewModeOverrides = false;
					Collector.AddMesh(ViewIndex, Mesh);

					/*GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Black, FString::Printf(TEXT("Vertex Num: %d, Index Num: %d"), VertexBuffer->Vertices.Num(), IndexBuffer->Indices.Num()));

					if (VertexBuffer->PositionBuffer.VertexBufferRHI.IsValid() && IndexBuffer->IndexBufferRHI.IsValid())
						GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Black, TEXT("缓冲区有效"));*/
						
					
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
					/*FMatrix TranslationMatrix = FTranslationMatrix(FVector(0.f, 0.f, 1000.f));
					FMatrix ToLocalMatrix = FScaleMatrix(FVector(1.f, 1.f, 1.f)) * TranslationMatrix;*/

					//DrawWireBox(pdi, FBox(FVector(0.f, 0.f, 0.f), FVector(100.f, 100.f, 100.f)), FColor::Black, 5, 0);
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

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = IsShown(View);
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = true;
			Result.bOpaqueRelevance = true;
			Result.bRenderInMainPass = ShouldRenderInMainPass();
			MaterialRelevance.SetPrimitiveViewRelevance(Result);

			return Result;
		}

		virtual bool CanBeOccluded() const override
		{
			return !MaterialRelevance.bDisableDepthTest;
		}

		virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
		uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

		//const FMaterialRenderProxy* RenderProxy;

private:
		FProcedureVertexBuffer* VertexBuffer;
		FProcedureIndexBuffer* IndexBuffer;
		FProcedureVertexFactory* VertexFactory;

		FMaterialRenderProxy* MaterialProxy;
		FMaterialRelevance MaterialRelevance;

		UBodySetup* MeshBody;
	};

	return new FProceduralSceneProxy(this);
}

FBoxSphereBounds UProcedureMesh::CalcBounds(const FTransform& LocalToWorld) const
{
	FBox Bound(ForceInit);

	TArray<FVector> TVertices =
	{
		FVector(0.f, 0.f, 0.f),
		FVector(200.f, 0.f, 0.f),
		FVector(200.f, 0.f, 110.f),
		FVector(200.f, 200.f, 0.f)
	};

	int32 VertexNum = TVertices.Num();
	if (TVertices.Num() > 0)
	{
		for (int32 i = 0; i < VertexNum; i++)
			Bound += TVertices[i];
	}

	return FBoxSphereBounds(Bound).TransformBy(LocalToWorld);
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
		NewBodySetup->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseSimpleAsComplex;    //使用简单碰撞，可用于物理模拟
		ModelSetup = NewBodySetup;
	}

	return ModelSetup;
}

UMaterialInterface* UProcedureMesh::GetMaterialFromCollisionFaceIndex(int32 FaceIndex, int32& SectionIndex) const
{
	return UMaterial::GetDefaultMaterial(MD_Surface);
}

bool UProcedureMesh::GetPhysicsTriMeshData(FTriMeshCollisionData * CollisionData, bool InUseAllTriData)
{
	TArray<FVector> Vertices =
	{
		FVector(0.f, 0.f, 0.f),
		FVector(200.f, 0.f, 0.f),
		FVector(200.f, 0.f, 110.f),
		FVector(200.f, 200.f, 0.f)
	};

	TArray<int32> Indices =
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

	CollisionData->Vertices = Vertices;
	for (int32 i = 0; i < Indices.Num(); i+=3)
	{
		FTriIndices Tri;
		Tri.v0 = Indices[i];
		Tri.v1 = Indices[i + 1];
		Tri.v2 = Indices[i + 2];
		CollisionData->Indices.Add(Tri);
	}

	CollisionData->bFlipNormals = true;
	CollisionData->bDeformableMesh = true;
	CollisionData->bFastCook = true;
	return true;
}

bool UProcedureMesh::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
{
	return true;
}

int32 UProcedureMesh::GetNumMaterials()const
{
	return 1;
}