// Fill out your copyright notice in the Description page of Project Settings.

#include "ProcedureMeshActor.h"
#include "ProcedureMesh.h"
#include "../Plugins/Runtime/ProceduralMeshComponent/Source/ProceduralMeshComponent/Public/ProceduralMeshComponent.h"


// Sets default values
AProcedureMeshActor::AProcedureMeshActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	DynamicMesh = nullptr;

	DynamicMesh = CreateDefaultSubobject<UProcedureMesh>(TEXT("DynamicMesh"));

	/*if (DynamicMesh)
		RootComponent = DynamicMesh;*/

	/*ProcedureMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcedureMesh"));

	TArray<FVector> Vertices =
	{
		FVector(0.f, 0.f, 0.f),
		FVector(20000.f, 0.f, 0.f),
		FVector(20000.f, 0.f, 11000.f),
		FVector(20000.f, 20000.f, 0.f)
	};

	TArray<int32> Indices=
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

	TArray<FVector> Normals=
	{
		FVector(0.f, 1.f, 0.f),
		FVector(0.f, 1.f, 0.f),
		FVector(0.f, 1.f, 0.f),
		FVector(0.f, 1.f, 0.f)
	};

	TArray<FVector2D> UV =
	{
		FVector2D(0.f,0.f),
		FVector2D(0.f,1.f),
		FVector2D(1.f,0.f),
		FVector2D(1.f,1.f)
	};

	TArray<FColor> Colors =
	{
		FColor::White,
		FColor::White,
		FColor::White,
		FColor::Black
	};

	TArray<FProcMeshTangent> TangentX =
	{
		FProcMeshTangent(0.f,0.f,1.f),
		FProcMeshTangent(1.f,0.f,1.f),
		FProcMeshTangent(0.f,1.f,0.f),
		FProcMeshTangent(1.f,1.f,1.f)
	};

	ProcedureMesh->CreateMeshSection(0, Vertices, Indices, Normals, UV, Colors, TangentX, false);*/
}
// Called when the game starts or when spawned
void AProcedureMeshActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProcedureMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

