// Fill out your copyright notice in the Description page of Project Settings.

#include "ProcedureMeshActor.h"
#include "ProcedureMesh.h"


// Sets default values
AProcedureMeshActor::AProcedureMeshActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	DynamicMesh = nullptr;

	DynamicMesh = CreateDefaultSubobject<UProcedureMesh>(TEXT("DynamicMesh"));

	if (DynamicMesh)
		RootComponent = DynamicMesh;
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

