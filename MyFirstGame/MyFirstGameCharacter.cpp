// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "MyFirstGameCharacter.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Classes/Animation/AnimInstance.h"
#include "Player/MyPlayerController.h"
#include "BoomActor.h"
#include "Weapon/Bullet.h"
#include "Weapon/Weapon_Gun.h"
#include "Kismet/GameplayStatics.h"
#include "MyFirstGame.h"
#include "Engine/Engine.h"
#include "Bonus.h"
#include "FlyObstacle.h"
#include "EngineUtils.h"
#include "RunPlatform.h"

//////////////////////////////////////////////////////////////////////////
// AMyFirstGameCharacter

AMyFirstGameCharacter::AMyFirstGameCharacter(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	//�Ƿ���뻷��״̬
	IsViewAround = false;
	OpenTheDoor = false;
	CloseTheDoor = false;

	CanShoot = true;
	IsInCrounch = false;    //��ʼ״̬��վ��״̬
	IsInCrounchToStand = false; //��ʼ״̬���ڶ�����
	IsInStandToCrounch = false;
	IsSmoothController = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = ObjectInitializer.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	
	CameraBoom->SetRelativeRotation(FRotator(45.0f, 0.0f, 0.0f));
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	CurrentWeaponType = EWeaponType::Type::Weapon_Instant;
	ShootInternal = 0.f;
	RunRate = 1.f; //Ĭ�ϼ��ٶ�����������

	//Ĭ����ҵ��ƶ��ٶ�
	MaxAcclerateSpeed = 850.f;
	MaxRunSpeed = 600.f;
	
	// Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	//FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMyFirstGameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("ViewAround", IE_Pressed, this, &AMyFirstGameCharacter::ViewAround);
	PlayerInputComponent->BindAction("ViewAround", IE_Released, this, &AMyFirstGameCharacter::StopViewAround);
	PlayerInputComponent->BindAction("OpenDoor", IE_Pressed, this, &AMyFirstGameCharacter::OperateDoor);
	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &AMyFirstGameCharacter::StartShoot);
	PlayerInputComponent->BindAction("Shoot", IE_Released, this, &AMyFirstGameCharacter::StopShoot);
	PlayerInputComponent->BindAction("Targeting", IE_Pressed, this, &AMyFirstGameCharacter::Targeting);
	PlayerInputComponent->BindAction("Targeting", IE_Released, this, &AMyFirstGameCharacter::StopTargeting);
	PlayerInputComponent->BindAction("ShootProjectile", IE_Released, this, &AMyFirstGameCharacter::Fire);
	PlayerInputComponent->BindAction("NextWeapon", IE_Released, this, &AMyFirstGameCharacter::NextWeapon);
	PlayerInputComponent->BindAction("PreWeapon", IE_Released, this, &AMyFirstGameCharacter::PreWeapon);
	PlayerInputComponent->BindAction("Accelerate", IE_Pressed, this, &AMyFirstGameCharacter::StartAccelerate);
	PlayerInputComponent->BindAction("Accelerate", IE_Released, this, &AMyFirstGameCharacter::StopAccelerate);
	PlayerInputComponent->BindAction("Crounch", IE_Pressed, this, &AMyFirstGameCharacter::ToggleCrounchStat);
	//PlayerInputComponent->BindAction("Crounch", IE_Released, this, &AMyFirstGameCharacter::EndCrounch);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyFirstGameCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyFirstGameCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &AMyFirstGameCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMyFirstGameCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMyFirstGameCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMyFirstGameCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AMyFirstGameCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AMyFirstGameCharacter::OnResetVR);
}

void AMyFirstGameCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CurMaxAcclerateSpeed = MaxAcclerateSpeed;
	CurMaxRunSpeed = MaxRunSpeed;

	WeaponNum = Inventory.Num();   //��������
	if (WeaponNum > 0)
	{
		for (int32 i = 0; i < WeaponNum; i++)
		{
			if (Inventory[i])
			{
				FActorSpawnParameters param;
				param.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;     //������ײ��
				AWeapon_Gun* weapon = GetWorld()->SpawnActor<AWeapon_Gun>(Inventory[i], param);
				InterInventory.Add(weapon);
			}
		}

		CurWeapon = InterInventory[0];
		EquipWeapon(CurWeapon);

		if (InterInventory[1] != nullptr)
			PackupWeapon(InterInventory[1]);   
	}
}

void AMyFirstGameCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AMyFirstGameCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AMyFirstGameCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AMyFirstGameCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	//AddYaw= Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds();
}

void AMyFirstGameCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMyFirstGameCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		if (IsViewAround)
		{
			// find out which way is right
			const FRotator Rotation = GetActorRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, Value);
		}
		//const FVector Direction1 = FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::X); 
		else
			// get forward vector
		{
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, Value);
		}
	}
}

void AMyFirstGameCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		if (!IsViewAround)
		{
			//use Controller
			// find out which way is right
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get right vector 
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			// add movement in that direction
			AddMovementInput(Direction, Value);

			//use Character
		}
	}
}

void AMyFirstGameCharacter::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (CanShoot && !GetCharacterMovement()->IsFalling())    //�������Ծ�Ĺ����в������
	{
		if (IsShooting)
		{	
			//if (!IsTargeting)
				//SetActorRotation(FRotator(0.f, GetControlRotation().Yaw, 0.f));     //�������ʱ�����������嶨������Ϊ���������򣬼�׼����ָ����

			float AnimRate = ComputeSuitRate(ShootSpeed);

			if (ShootInternal == 0)
			{
				Shoot(AnimRate);
			}
			ShootInternal += DeltaTime;       //
			if (ShootInternal >= 1.f / ShootSpeed)
			{
				Shoot(AnimRate);
				ShootInternal = 0.01f;
			}
		}
		else
		{
			ShootInternal = 0.f;
		}
	}

	if (IsInStandToCrounch)       //96-48
	{
		if (GetWorld()->GetTimeSeconds() - CurActionTime >= ActionAnimTime)
		{
			IsInStandToCrounch = false;
		}
		
	}
	if (IsInCrounchToStand)       //48-96
	{
		if (GetWorld()->GetTimeSeconds() - CurActionTime >= ActionAnimTime)
		{
			IsInCrounchToStand = false;
		}
	}

	if (IsInAccelerate && !IsInCrounch && !IsTargeting)
		GetCharacterMovement()->MaxWalkSpeed = CurMaxAcclerateSpeed;       //���ٶȱ�Ϊ��ǰ����״̬�ٶ�

	else if (IsInCrounch || IsTargeting)
		GetCharacterMovement()->MaxWalkSpeed = 250;    //�¶׺���׼ʱ���ٶ�

	else
		GetCharacterMovement()->MaxWalkSpeed = MaxRunSpeed;      //�ٶȸ�Ϊ��ǰ�����ܲ��ٶ�

	//�������ж��Ƿ���Կ�ǹ�������ٶ���ֵ�жϣ�����800�ͻᲥ�ż��ٶ���������״̬�������ã����ڿ������һ��������ֵ�ӿڸ�״̬����Ҳ�Ͳ��ܿ�ǹ��
	if (GetCharacterMovement()->MaxWalkSpeed >= 800.f)
		CanShoot = false;
	else
		CanShoot = true;

	//�ڻ�����ת�ӽǺ�Controllerƽ�����ɵ����(Actor)����
	if (IsSmoothController && Controller != NULL)
	{
		FVector ControllerDir = Controller->GetControlRotation().Vector().GetSafeNormal2D();
		FVector CharDir = GetActorRotation().Vector().GetSafeNormal2D();
		float SubDegree = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CharDir, ControllerDir)));      //��ȡ�������Ķ���

		float CharYaw = AdjustDegree(GetActorRotation().Yaw);
		float ControllerYaw = AdjustDegree(Controller->GetControlRotation().Yaw);

		float DeltaDegree = 0.f;
		if (AdjustDegree(ControllerYaw + SubDegree) > CharYaw - 1.f && AdjustDegree(ControllerYaw + SubDegree) < CharYaw + 1.f)
			DeltaDegree = SubDegree;
		else
			DeltaDegree = -SubDegree;

		float NewControllerYaw = FMath::FInterpTo(ControllerYaw, ControllerYaw + DeltaDegree, DeltaTime, 10.f);
		Controller->SetControlRotation(FRotator(0.f, AdjustDegree(NewControllerYaw), 0.f));

		if (NewControllerYaw >= ControllerYaw + DeltaDegree - 5 && NewControllerYaw <= ControllerYaw + DeltaDegree + 5)
		{
			IsViewAround = false;
			IsSmoothController = false;
		}
	}

	//GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Blue, FString::Printf(TEXT("Player Speed:%f"), GetCharacterMovement()->Velocity.Size()), true);
}

void AMyFirstGameCharacter::Destroyed()
{
	int32 WeaponNum = InterInventory.Num();      //ɾ�����ɵ�����
	for (int32 i = 0; i < WeaponNum; i++)
	{
		if (InterInventory[i] != nullptr)
		{
			InterInventory[i]->Destroy();
			InterInventory[i] = nullptr;
		}
	}

	for (TActorIterator<AFlyObstacle> It(GetWorld()); It; ++It)
	{
		(*It)->DestroyActor();
	}

	AMyPlayerController* MPC = Cast<AMyPlayerController>(Controller);
	if (MPC != nullptr)
	{
		int32 PlatNum = MPC->PlatformArray.Num();
		for (int32 i = 0; i < PlatNum; ++i)
		{
			MPC->PlatformArray[i]->DestroyActor();
			MPC->PlatformArray[i] = nullptr;
		}
	}

	Super::Destroyed();
}

void AMyFirstGameCharacter::AddControllerYawInput(float Val)
{
	if (!IsSmoothController)
		Super::AddControllerYawInput(Val);

		if (Val > 0)
			IsToRight = true;

		else if (Val < 0)
			IsToRight = false;
}
void AMyFirstGameCharacter::ViewAround()
{
	CanShoot = false;       //���˵�ʱ�������
	IsViewAround = true;
}

void AMyFirstGameCharacter::StopViewAround()
{
	CanShoot = true;
	IsSmoothController = true;
}

void AMyFirstGameCharacter::StartAccelerate()
{
	if (!IsInCrounch)       //���¶׵�ʱ���ܼ���
		IsInAccelerate = true;       //�����ܵ�ʱ��Ҳ�������
}

void AMyFirstGameCharacter::StopAccelerate()
{
	IsInAccelerate = false;
}

FRotator AMyFirstGameCharacter::ComputeAimOffset()const
{
	//��һ�ַ���
	if (Controller)                    //ע�����ؼӣ�������ͼ������һ��Ҫ�ж�ָ���Ƿ�Ϊ�� GetBaseAimRotation() �����Ѿ��жϹ�
	{
		const FRotator deltaR = /*GetBaseAimRotation()*/Controller->GetControlRotation() - GetActorRotation();
		const float YawDelta = AdjustDegree(deltaR.Yaw);
		const float PitchDelta = AdjustDegree(deltaR.Pitch);
		const FRotator AimOffset = FRotator(PitchDelta, YawDelta, 0.f);

		return AimOffset; 
	}
	return FRotator::ZeroRotator;

	//�ڶ��ַ���
	//const FVector AimDirWS = GetBaseAimRotation().Vector();
	//const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);    //��ȡ������תVector�Ĳ����offset
	//const FRotator AimRotLS = AimDirLS.Rotation();

	//return AimRotLS;
}

void AMyFirstGameCharacter::OperateDoor()
{
	if (IsInDoorCollision)
	{
		if (IsInOpen)
		{
			CloseTheDoor = true;
			OpenTheDoor = false;
		}
		else
		{
			OpenTheDoor = true;
			CloseTheDoor = false;
		}
	}
}

void AMyFirstGameCharacter::AvoidOpenDoor()
{
	IsInDoorCollision = false;
	OpenTheDoor = false;
	CloseTheDoor = false;
}

void AMyFirstGameCharacter::UpdateStandCharacter()
{
	float sub = 0.f;					 //Controller��Character  ��Yaw��֮��Ĳ�
	if (Controller)
	{
		float ControllerCurYaw = Controller->GetControlRotation().Yaw;
		float CharacterYaw = GetActorRotation().Yaw;

		FVector ControllerVec = FRotator(0.f, ControllerCurYaw, 0.f).Vector().GetSafeNormal();
		FVector CharacterVec = GetActorRotation().Vector().GetSafeNormal();
		float SubAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ControllerVec, CharacterVec)));
	
		if (SubAngle >= 80.f)
		{
			if (!IsViewAround)
			{
				if (IsToRight)
				{
					SetActorRotation(FRotator(0.f, ControllerCurYaw - 80.f, 0.f));
				}
				else
				{
					SetActorRotation(FRotator(0.f, ControllerCurYaw + 80.f, 0.f));
				}
			}
		}
		//GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Blue, FString::Printf(TEXT("Player Speed:%f"), GetCharacterMovement()->Velocity.Size()), true);
	}
}

float AMyFirstGameCharacter::AdjustDegree(const float in)const
{
	if (in > 180.f)
	{
		return in - 360.f;
	}
	if (in < -180.f)
	{
		return in + 360.f;
	}
		return in;
}

void AMyFirstGameCharacter::Shoot(float AimRate)
{
	//�Ȳ����������
	if (IsTargeting && ShootAim_Ironsight)
		PlayAnim(AimRate, ShootAim_Ironsight);
	if (!IsTargeting && ShootAim)
		PlayAnim(AimRate, ShootAim);

	Fire();
}

FHitResult AMyFirstGameCharacter::QueryCrossHair(float Distance)
{
	AMyPlayerController* MPC;
	FVector ViewLocation;
	FRotator ViewRotation;
	FHitResult Result;

	if (Controller)
	{
		MPC = Cast<AMyPlayerController>(Controller);
		ViewLocation = MPC->PlayerCameraManager->ViewTarget.POV.Location;
		ViewRotation = MPC->GetControlRotation();
		//FVector ForwardVec = ViewRotation.Vector();                    //Ҳʹ�ø÷���
		FVector ForwardVec = FRotationMatrix(ViewRotation).GetUnitAxis(EAxis::X);         //��ȡĳ����ķ���Ҳ����ʹ�þ���

		FCollisionQueryParams TraceParam(TEXT("QueryInstant"), false, this);
		FCollisionResponseParams ResponseParam(ECollisionResponse::ECR_Block);

		GetWorld()->LineTraceSingleByChannel(Result, ViewLocation, ViewLocation + Distance*ForwardVec, ECollisionChannel::ECC_Visibility, TraceParam, ResponseParam);
	}
	return Result;
}

FVector AMyFirstGameCharacter::ComputeShootDir(float AdjustDistance)
{
	AMyPlayerController* MPC;
	FVector ViewLocation;
	FRotator ViewRotation;
	
	if (Controller)
	{
		MPC = Cast<AMyPlayerController>(Controller);
		ViewLocation = MPC->PlayerCameraManager->ViewTarget.POV.Location;
		ViewRotation = MPC->GetControlRotation();
		
		FVector ForwardVec = FRotationMatrix(ViewRotation).GetUnitAxis(EAxis::X);         //��ȡĳ����ķ���Ҳ����ʹ�þ���
		FVector ShootVec = ForwardVec * AdjustDistance + ViewLocation - CurWeapon->GetFireLocation();    //ǹ��λ�õ��������������һ������λ�õķ���

		return ShootVec;
	}
	return FVector::ZeroVector;
}

void AMyFirstGameCharacter::Targeting()
{
		IsTargeting = true;
}

void AMyFirstGameCharacter::StopTargeting()
{
	IsTargeting = false;
}

void AMyFirstGameCharacter::Fire()
{
	FVector ShootDir;
	FHitResult Result = QueryCrossHair(10000.f);
	if (Cast<ABoomActor>(Result.GetActor()))
	{
		ABoomActor* Aim = Cast<ABoomActor>(Result.GetActor());
		ShootDir = (Aim->GetActorLocation() - CurWeapon->GetFireLocation()).GetSafeNormal();
	}
	else
	{
		//ShootDir = (Controller->GetControlRotation() + FRotator(2.f, 0.f, 0.f)).Vector();      //Pitch��ת�����2�ȣ�������׼��λ�ã��������ʵ�ⲻ�����У�ƫ�Ʋ��ÿ���
		ShootDir = ComputeShootDir(50000.f);     //������ʵ�����Ƕȣ���������λ500�׵�������
	}
	CurWeapon->Fire(ShootDir);
}

void AMyFirstGameCharacter::NextWeapon()
{
	int32 NextWeaponIndex = (InterInventory.Find(CurWeapon) + 1) % InterInventory.Num();
	EquipWeapon(InterInventory[NextWeaponIndex]); //װ����һ������
}

void AMyFirstGameCharacter::PreWeapon()
{
	int32 PreWeaponIndex = InterInventory.Find(CurWeapon) == 0 ? (InterInventory.Num() - 1) : (InterInventory.Find(CurWeapon) - 1) % InterInventory.Num();
	EquipWeapon(InterInventory[PreWeaponIndex]); //װ��ǰһ������
}

float AMyFirstGameCharacter::PlayAnim(float rate, UAnimMontage* Anim)
{
	USkeletalMeshComponent* PlayerMesh = GetMesh();
	if (PlayerMesh && Anim && PlayerMesh->AnimScriptInstance)
	{
		return PlayerMesh->AnimScriptInstance->Montage_Play(Anim, rate);
	}
	return 0;
}

float AMyFirstGameCharacter::ComputeSuitRate(int8 CurShootSpeed)
{
	
	if (CurrentWeaponType == EWeaponType::Weapon_Beam)   //���������ǹ�Ͱ���������Ƶ��
		return 1.f;

	else if (CurrentWeaponType == EWeaponType::Weapon_Projectile)
	{
		float PerShootAnimTime = 0.233f;    //ÿ��������ʱ��
		float CurPerShootTime = 1.f / CurShootSpeed;          //��ǰ��������£������ӵ����ʱ��
		return PerShootAnimTime / CurPerShootTime;
	}
	return 1.f;
}

void AMyFirstGameCharacter::ToggleCrounchStat()
{
	if (!IsInStandToCrounch && !IsInCrounchToStand)             
	{
		//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green,"In Toggle");
		IsInCrounch = IsInCrounch + 1;

		if (IsInCrounch)
		{
			if (IsTargeting && StandToCrounchAim_Ironsight)
				ActionAnimTime = PlayAnim(1.f, StandToCrounchAim_Ironsight);
			if (!IsTargeting && StandToCrounchAim)
				ActionAnimTime = PlayAnim(1.f, StandToCrounchAim); //�������ŵ�ʱ��

			CurActionTime = GetWorld()->GetTimeSeconds();          //��ȡ������ʼʱ��
			IsInStandToCrounch = true;
			GetCharacterMovement()->MaxWalkSpeed = 250.f;

			FVector CapLocation = GetCapsuleComponent()->GetComponentLocation();
			GetCapsuleComponent()->SetCapsuleHalfHeight(48.f);     //������£���ײ��ҲҪ���ű�С��������СΪԭ����һ��
			GetCapsuleComponent()->SetWorldLocation(CapLocation - FVector(0.f, 0.f, 48.f));
			GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -48));
		}
		else 
		{
			if (IsTargeting && CrounchToStandAim_Ironsight)
				ActionAnimTime = PlayAnim(1.f, CrounchToStandAim_Ironsight);
			if (!IsTargeting && CrounchToStandAim)
				ActionAnimTime = PlayAnim(1.f, CrounchToStandAim);

			CurActionTime = GetWorld()->GetTimeSeconds();
			IsInCrounchToStand = true;
			GetCharacterMovement()->MaxWalkSpeed = MaxRunSpeed;

			FVector CapLocation = GetCapsuleComponent()->GetComponentLocation();
			GetCapsuleComponent()->SetCapsuleHalfHeight(96.f);
			GetCapsuleComponent()->SetWorldLocation(CapLocation + FVector(0.f, 0.f, 48.f));
			GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -96));
		}
	}
}

void AMyFirstGameCharacter::EquipWeapon(AWeapon_Gun* curWeapon)
{
	//���õ�ǰ�������ͣ�������
	if (curWeapon)
	{
		CurrentWeaponType = curWeapon->WeaponData.WeaponType;
		AMyPlayerController* MPC = Cast<AMyPlayerController>(Controller);
		if (MPC != nullptr)
			MPC->ChangeWeaponType(CurrentWeaponType);     //���������Ͳ�������Controller

		this->ShootSpeed = curWeapon->WeaponData.ShootSpeed;

		this->CurWeapon->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
		PackupWeapon(CurWeapon);
		this->CurWeapon = curWeapon;
		CurWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WeaponSocket);       //��Weapon���ŵ��������,��ס�Ǹ����������ϣ�����������Actor
		this->CurWeapon->SetActorRelativeRotation(FRotator(0.f, 90.f, 0.f));
	}
}

void AMyFirstGameCharacter::PackupWeapon(AWeapon_Gun* PackupWeapon)
{
	/*Ĭ�������е�һ��Ϊһ���������ڶ���Ϊ��������*/
	int32 AttackIndex = InterInventory.Find(PackupWeapon);
	if (AttackIndex == 0)
	{
		PackupWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FirstWeaponSocket);
		PackupWeapon->SetActorRelativeRotation(FRotator(90.f, 0.f, 90.f));
	}
	else if (AttackIndex == 1)
	{
		PackupWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, SecondWeaponSocket);
		PackupWeapon->SetActorRelativeRotation(FRotator(90.f, 0.f, 90.f));
	}
}

void AMyFirstGameCharacter::ApplyBonus(class ABonus* BonusActor)
{
	EBonusType::Type BonusType = BonusActor->BonusData.BonusType;    //��ȡ�ý���״̬
	if (BonusType != EBonusType::Bonus_None)
	{
		if (BonusType == EBonusType::Bonus_Score)
			AddScore(BonusActor->GetBonusScore());

		else if (BonusType == EBonusType::Bonus_Accelerate)
			AddSpeed(BonusActor->GetBonusSpeed());

		else if (BonusType == EBonusType::Bonus_NoObstacle)
		{
			AMyPlayerController* MPC = Cast<AMyPlayerController>(Controller); 
			MPC->StartToAll(BonusActor->BonusData.NoObstacleTime);
		}
	}
}

void AMyFirstGameCharacter::AddScore(int32 BonusScore)
{
	//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::FormatAsNumber(BonusScore), true);
}

void AMyFirstGameCharacter::AddSpeed(int32 BonusSpeed)
{

}