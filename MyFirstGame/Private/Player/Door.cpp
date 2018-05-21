// Fill out your copyright notice in the Description page of Project Settings.

#include "Door.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "MyFirstGameCharacter.h"

// Sets default values
ADoor::ADoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bGenerateOverlapEventsDuringLevelStreaming = true;
	DoorCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorCollision"));
	DoorRoot = CreateDefaultSubobject<UArrowComponent>(TEXT("DoorRoot"));
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));

	RootComponent = DoorCollision;
	DoorCollision->SetWorldScale3D(FVector(5.0f, 5.0f, 5.0f));

	const FVector CollisionExtent = DoorCollision->GetScaledBoxExtent();

	DoorRoot->SetupAttachment(RootComponent);
	DoorRoot->SetRelativeLocation(FVector(-CollisionExtent.GetComponentForAxis(EAxis::X) / 10.0f, 0.0f, 0.0f));

	if (DoorMesh)
	{
		DoorMesh->SetupAttachment(DoorRoot);
	}

	//门默认为关
	IsInOpen = false;
	bReset = false;
}

// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CanbOperate)
	{
		if (CurChar->OpenTheDoor)   //如果为true就可以进行开门
		{
			IsInOpen = true;
			IsInCloseRotate = false;
			YawRotation = FMath::FInterpTo(DoorRoot->RelativeRotation.Yaw, 120.f, DeltaTime, 4.f);
			DoorRoot->SetRelativeRotation(FRotator(0.0f, YawRotation, 0.0f));
			
			if (YawRotation == 120.f)
				IsInOpenRotate = false;
			else
				IsInOpenRotate = true;
		}
		if(CurChar->CloseTheDoor)
		{
			IsInOpen = false;
			IsInOpenRotate = false;
			YawRotation = FMath::FInterpTo(DoorRoot->RelativeRotation.Yaw, 0.f, DeltaTime, 4.f);
			DoorRoot->SetRelativeRotation(FRotator(0.0f, YawRotation, 0.0f));

			if (YawRotation == 0.f)
				IsInCloseRotate = false;
			else
				IsInCloseRotate = true;
		}
		CurChar->IsInOpen = IsInOpen;
	}
	else
	{
		if (IsInOpenRotate)
		{
			YawRotation = FMath::FInterpTo(DoorRoot->RelativeRotation.Yaw, 120.f, DeltaTime, 4.f);
			DoorRoot->SetRelativeRotation(FRotator(0.0f, YawRotation, 0.0f));

			if (YawRotation == 120.f)
				IsInOpenRotate = false;
		}
		if (IsInCloseRotate)
		{
			YawRotation = FMath::FInterpTo(DoorRoot->RelativeRotation.Yaw, 0.f, DeltaTime, 4.f);
			DoorRoot->SetRelativeRotation(FRotator(0.0f, YawRotation, 0.0f));

			if (YawRotation == 0.f)
				IsInCloseRotate = false;
		}
	}

	if (bReset)
	{
		IsInOpen = false;
		IsInOpenRotate = false;
		YawRotation = FMath::FInterpTo(DoorRoot->RelativeRotation.Yaw, 0.f, DeltaTime, 4.f);
		DoorRoot->SetRelativeRotation(FRotator(0.0f, YawRotation, 0.0f));
		UE_LOG(LogRunGame, Log, TEXT("Door Tick"))

		if (FMath::Abs(YawRotation) < 1.f)
		{
			DoorRoot->SetRelativeRotation(FRotator(0.0f, 0.f, 0.0f));
			bReset = false;   //门重置完成
			IsInCloseRotate = false;
		}
	}
}

void ADoor::NotifyActorBeginOverlap(AActor* OtherActor)
{ 
	if (Cast<AMyFirstGameCharacter>(OtherActor))
	{
		CurChar = Cast<AMyFirstGameCharacter>(OtherActor);
		CurChar->IsInDoorCollision = true;
		CurChar->IsInOpen = IsInOpen;
		CanbOperate = true;
	}
}

void  ADoor::NotifyActorEndOverlap(AActor* OtherActor)
{
	if (CurChar != NULL && CurChar == Cast<AMyFirstGameCharacter>(OtherActor))
	{
		CurChar->AvoidOpenDoor();
		CurChar = NULL;
		CanbOperate = false;
	}
}

