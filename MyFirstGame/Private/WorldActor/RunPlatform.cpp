// Fill out your copyright notice in the Description page of Project Settings.

#include "RunPlatform.h"


// Sets default values
ARunPlatform::ARunPlatform(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ARunPlatform::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

// Called every frame
void ARunPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

