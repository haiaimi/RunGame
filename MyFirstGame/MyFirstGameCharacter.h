// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyFirstGame.h"
#include "MyFirstGameCharacter.generated.h"

class ABonus;
class AFlyObstacle;

//默认玩家的移动速度
const float MaxAcclerateSpeed = 850.f;
const float MaxRunSpeed = 600.f;

UCLASS(config=Game)
class AMyFirstGameCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<class ABullet> ProjectileWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TArray<TSubclassOf<class AWeapon_Gun>> Inventory;

	//用来存放当前武器库的指针数组
	UPROPERTY()
	TArray<class AWeapon_Gun*> InterInventory;

	/**放武器的插槽*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName WeaponSocket;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName FirstWeaponSocket;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName SecondWeaponSocket;

	class AWeapon_Gun* CurWeapon;

	/**武器数量*/
	int32 WeaponNum;

	UPROPERTY(EditDefaultsOnly, Category = "Aim_Motage")
	UAnimMontage* ShootAim;

	UPROPERTY(EditDefaultsOnly, Category = "Aim_Motage")
	UAnimMontage* ShootAim_Ironsight;

	UPROPERTY(EditDefaultsOnly, Category = "Aim_Motage")
	UAnimMontage* StandToCrounchAim;

	UPROPERTY(EditDefaultsOnly, Category = "Aim_Motage")
	UAnimMontage* CrounchToStandAim;

	/**下面得是瞄准时得动作*/
	UPROPERTY(EditDefaultsOnly, Category = "Aim_Motage")
	UAnimMontage* CrounchToStandAim_Ironsight;

	UPROPERTY(EditDefaultsOnly, Category = "Aim_Motage")
	UAnimMontage* StandToCrounchAim_Ironsight;

	/**这里主要是修改玩家加速时动画的播放速率*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aim_Motage")
	float RunRate;

	/**是否在向左转*/
	uint8 IsToRight : 1;

	/**是否在环顾*/
	uint8 IsViewAround : 1;

	/**开门*/
	uint8 OpenTheDoor : 1;

	/**关门*/
	uint8 CloseTheDoor : 1;

	/**是否在下蹲状态*/
	uint8 IsInCrounch : 1;

	/**是否处于站立到下蹲动作之中*/
	uint8 IsInStandToCrounch : 1;

	/**是否处于下蹲到站立动作之中*/
	uint8 IsInCrounchToStand : 1;

	/**是否在加速*/
	uint8 IsInAccelerate : 1;

	/**玩家移动中变化后的速度（在平台上）*/
	float CurMaxRunSpeed, CurMaxAcclerateSpeed;

	/**站立、蹲下动画时间*/
	float ActionAnimTime = 0.f;

	float CurActionTime;
	
	/**是否在门的检测区内*/
	uint8 IsInDoorCollision : 1;

	/**检测区域内的门处于开启状态*/
	uint8 IsInOpen : 1;

	/**检测区域内的门处于关闭状态*/
	uint8 IsInClose : 1;

	/***/
	float AddYaw = 0.f;

	/**玩家是否在瞄准状态*/
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Anim_Space")
	uint8 IsTargeting : 1;

	/**玩家当前武器*/
	TEnumAsByte<EWeaponType::Type> CurrentWeaponType;

	/**玩家当前是否在射击*/
	uint8 IsShooting : 1;

	/**当前武器射速*/
	uint8 ShootSpeed;

	/**武器射击的间隔*/
	float ShootInternal;

	/**玩家能否开枪*/
	uint8 CanShoot : 1;

	/**是否开始平滑恢复Controller的方向，这是在环顾后视角发生变化时的操作*/
	uint8 IsSmoothController : 1;

	/**速度增量，可能会用于速度提升奖励*/
	float AddedSpeed;

protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/**进入环顾四周状态*/
	void ViewAround();

	/**停止环顾*/
	void StopViewAround();

	/**玩家进入加速状态*/
	void StartAccelerate();

	/**停止加速*/
	void StopAccelerate();

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	/**玩家生成/游戏执行时所执行的操作*/
	virtual void PostInitializeComponents()override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable,Category = "Pawn")
	FRotator ComputeAimOffset()const;

	virtual void AddControllerYawInput(float del);

	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction);

	virtual void Destroyed()override;

	///玩家动作相关，开门、观察视角
	void UpdateStandCharacter();

	bool IsCharacterViewAround()
	{
		return IsViewAround;
	}
	/**操作门*/
	void OperateDoor();

	/**离开门检测区后禁止开门行为*/
	void AvoidOpenDoor();

	void ToggleCrounchStat();


	//用于动画状态机
	UFUNCTION(BlueprintPure, Category = "Pawn")
	bool IsPlayerInCrounch() { return IsInCrounch; }

	FORCEINLINE	float AdjustDegree(const float in)const;
	
	///下面是玩家射击相关内容
	void StartShoot() { IsShooting = true; }

	void StopShoot() { IsShooting = false; }

	/**
	 *  检测当前准心是否对准目标
	 *  @Param Distance 检测的距离
	 */
	FHitResult QueryCrossHair(float Distance);

	/**射击*/
	void Shoot(float AimRate);

	/**进入瞄准状态*/
	void Targeting();

	/**停止瞄准*/
	void StopTargeting();

	bool PlayerIsTargeting() { return IsTargeting; }

	/**发射抛射物子弹*/
	void Fire();

	/**选则武器*/
	void NextWeapon();

	void PreWeapon();

	void EquipWeapon(AWeapon_Gun* const CurWeapon);

	/*把武器收起放到背后*/
	void PackupWeapon(AWeapon_Gun* PickupWeapon);

	///下面是动画相关
	/**播放蒙太奇动画，返回动画时间*/
	float PlayAnim(float rate, UAnimMontage* Anim);

	float ComputeSuitRate(int8 CurShootSpeed);

	/**第三人称中的射击容易偏移，这里就计算射击的角度，最大程度的与准心适配*/
	FVector ComputeShootDir(float AdjustDistance);

	///下面是玩家的奖励如分数、速度
	void ApplyBonus(class ABonus* BonusActor);

	void AddScore(class ABonus* BonusActor);

	void AddSpeed(int32 BonusSpeed);

};

