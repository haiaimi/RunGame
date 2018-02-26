// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerCameraManager.h"
#include "MyFirstGameCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

AMyPlayerCameraManager::AMyPlayerCameraManager(const FObjectInitializer& Initializer):Super(Initializer)
{
	bAlwaysApplyModifiers = true;
	ViewPitchMin = -70.f;
	ViewPitchMax = 70.f;
	FirstViewFOV = 60.f;
}

void AMyPlayerCameraManager::UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime)
{
	FVector ViewLoc;
	FRotator ViewRot;
	OutVT.Target->GetActorEyesViewPoint(ViewLoc, ViewRot);

	//FMatrix CameraTranslation = FTranslationMatrix(ViewLoc)*FTranslationMatrix(FVector(-45.0f, 0.0f, 15.0f));
	//FMatrix CameraTransform = FRotationMatrix(ViewRot)*CameraTranslation;
	FMatrix ViewRotation = FRotationMatrix(ViewRot);
	
	//ViewLoc += ViewRotation.GetUnitAxis(EAxis::X)*-90.0f;
	ViewLoc += ViewRotation.GetUnitAxis(EAxis::Y)*40.f;
	ViewLoc += ViewRotation.GetUnitAxis(EAxis::Z)*20.f;
	
	if (Cast<AMyFirstGameCharacter>(OutVT.Target))
	{
		AMyFirstGameCharacter* MyCharacter = Cast<AMyFirstGameCharacter>(OutVT.Target);

		UCharacterMovementComponent* MyMoveComponent = MyCharacter->GetCharacterMovement();
		if (MyMoveComponent->Velocity.Size() == 0)
		{
			MyCharacter->bUseControllerRotationYaw = false;
			MyCharacter->UpdateStandCharacter();
		}
		if (MyMoveComponent->Velocity.Size() > 0 && !MyCharacter->IsCharacterViewAround())
		{
			MyCharacter->bUseControllerRotationYaw = true;
		}

		if (MyCharacter->IsCharacterViewAround())
		{
			MyCharacter->bUseControllerRotationYaw = false;
			ViewLoc = FMath::VInterpTo(OutVT.POV.Location, ViewLoc + ViewRotation.GetUnitAxis(EAxis::X)*-180.0f, DeltaTime, 30);       
			if (DefaultFOV != 90.f)
				DefaultFOV = FMath::FInterpTo(DefaultFOV, 90.f, DeltaTime, 5.f);
		}
		else
		{
			//ViewLoc = FMath::VInterpTo(OutVT.POV.Location, ViewLoc + ViewRotation.GetUnitAxis(EAxis::X)*-120.0f, DeltaTime, 30);  //该效果会造成画面抖动
			ViewLoc += ViewRotation.GetUnitAxis(EAxis::X)*-120.0f;
			if (MyCharacter->PlayerIsTargeting())
			{
				//ViewLoc = FMath::VInterpTo(OutVT.POV.Location, ViewLoc + ViewRotation.GetUnitAxis(EAxis::X)*30.0f, DeltaTime, 30);
				if (DefaultFOV != 80.f)
					DefaultFOV = FMath::FInterpTo(DefaultFOV, 75.f, DeltaTime, 5.f);
			}
			else
			{
				if (DefaultFOV != 90.f)
					DefaultFOV = FMath::FInterpTo(DefaultFOV, 90.f, DeltaTime, 5.f);
			}
		}
	}
		
	OutVT.POV.Location = ViewLoc;
	OutVT.POV.Rotation = FMath::RInterpTo(OutVT.POV.Rotation, ViewRot, DeltaTime, 50.0f);
}

