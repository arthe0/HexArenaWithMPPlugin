// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HATypes/TurningInPlace.h"
#include "Interfaces/InteractWithCrosshairsInterface.h"
#include "HAComponents/CombatComponent.h"
#include "HAComponents/HealthComponent.h"
#include "Components/TimelineComponent.h"
#include "PlayerStates/HaPlayerState.h"
#include "HATypes/CombatState.h"
#include "HAComponents/HitBoxComponent.h"
#include "HABaseCharacter.generated.h"



class UCombatComponent;
class UHealthComponent;
class UCameraComponent;
class UWidgetComponent;
class ABaseWeapon;
class ABasePickup;
class UAnimMontage;
class AHAPlayerController;
class UTimelineComponent;
class AHaPlayerState;
class UBoxComponent;
class ULagCompensationComponent;
class UHitBoxComponent;
class UHAMovementComponent;
class UInventory;

UCLASS()
class HEXARENA_API AHABaseCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	AHABaseCharacter(const FObjectInitializer& ObjInit);
	
	friend UHealthComponent;

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	void PlayFireMontage(bool bAiming);
	void PlayDeathMontage();
	void PlayReloadMontage();

	void Death();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDeath();

	AHAPlayerController* HAPlayerController;

	UPROPERTY()
	TMap<FName, UHitBoxComponent*> HitBoxes;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Mesh")
	USkeletalMeshComponent* ClientMesh;

protected:

	bool bIsMovingForward = false;

	virtual void BeginPlay() override;

	void MoveForward(float Amount);
	void MoveRight(float Amount);
	void LookUp(float Amount);
	void TurnAround(float Amount);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void CrouchButtonReleased();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed();
	void FireButtonReleased();
	void ReloadButtonPressed();
	void SprintButtonPressed();
	void SprintButtonReleased();
	void AimOffset(float DeltaTime);

	//	Poll for any relevant class and init HUD
	void PollInit();

	/*
	 * Hit Boxes for server side rewind
	 */

	UPROPERTY(EditAnywhere)
	UHitBoxComponent* HeadBox;

	UPROPERTY(EditAnywhere)
	UHitBoxComponent* NeckBox;

	UPROPERTY(EditAnywhere)
	UHitBoxComponent* ChestBox;

	UPROPERTY(EditAnywhere)
	UHitBoxComponent* StomachBox;

	UPROPERTY(EditAnywhere)
	UHitBoxComponent* PelvisBox;

	UPROPERTY(EditAnywhere)
	UHitBoxComponent* UpperArmLBox;

	UPROPERTY(EditAnywhere)
	UHitBoxComponent* UpperArmRBox;

	UPROPERTY(EditAnywhere)
	UHitBoxComponent* LowerArmLBox;

	UPROPERTY(EditAnywhere)
	UHitBoxComponent* LowerArmRBox;

	UPROPERTY(EditAnywhere)
	UHitBoxComponent* HandLBox;

	UPROPERTY(EditAnywhere)
	UHitBoxComponent* HandRBox;

	UPROPERTY(EditAnywhere)
	UHitBoxComponent* ThighLBox;

	UPROPERTY(EditAnywhere)
	UHitBoxComponent* ThighRBox;

	UPROPERTY(EditAnywhere)
	UHitBoxComponent* CalfLBox;

	UPROPERTY(EditAnywhere)
	UHitBoxComponent* CalfRBox;

	UPROPERTY(EditAnywhere)
	UHitBoxComponent* FootLBox;

	UPROPERTY(EditAnywhere)
	UHitBoxComponent* FootRBox;

private:
	/*
	*  Pickups 
	*/

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingPickup)
	ABasePickup* OverlappingPickup;

	UFUNCTION()
	void OnRep_OverlappingPickup(ABasePickup* LastPickup);

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION()
	void EquipWeaponHandle(ABasePickup* Pickup);

	UFUNCTION()
	void EquipAmmoHandle(ABasePickup* Pickup);

	UFUNCTION()
	void EquipPowerUpHandle(ABasePickup* Pickup);

	/*
	 *	HA Components 
	 */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere)
	UHealthComponent* Health;

	UPROPERTY(VisibleAnywhere)
	ULagCompensationComponent* LagCompensation;

	UPROPERTY(VisibleAnywhere)
	UInventory* Inventory;

	/*
	* Montages
	*/

	UPROPERTY(EditAnywhere, Category = "Montages")
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Montages")
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = "Montages")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = "Montages")
	UAnimMontage* DeathMontage;

	void PlayHitReactMontage();


	float ADSWeight = 0.f;
	float AO_Yaw;
	float AO_Pitch;
	bool bDeath = false;

	FTimerHandle DeathTimer;

	UPROPERTY(EditDefaultsOnly)
	float DeathDelay = 5.f;

	void DeathTimerFinished();

	FRotator StartingAimRoation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	/*
	* Dissolve effect
	*/	

	//Dynamic instances changes during runtime
	UPROPERTY(VisibleAnywhere, Category = "Death")
	UMaterialInstanceDynamic* DynamicBodyDissolveMaterialInstance;

	UPROPERTY(VisibleAnywhere, Category = "Death")
	UMaterialInstanceDynamic* DynamicEquipDissolveMaterialInstance;

	//Material instances set on the BP, used with dynamic materials
	UPROPERTY(EditAnywhere, Category = "Death")
	UMaterialInstance* BodyDissolveMaterialInstance;

	UPROPERTY(EditAnywhere, Category = "Death")
	UMaterialInstance* EquipDissolveMaterialInstance;

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	AHaPlayerState* HAPlayerState;
public:
	void SetOverlappingPickup(ABasePickup* Pickup);
	bool IsWeaponEquipped();
	bool IsAiming();

	float GetMovementDirection();

	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE UCameraComponent* GetCameraComponent() const { return CameraComponent; }
	FORCEINLINE UHealthComponent* GetHealthComponent() const { return Health; }

	//MB not a good idea???
	FORCEINLINE float GetADSWeight() { return Combat->ADSWeight; }
	ABaseWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }

	//Is it a good idea?
	FORCEINLINE bool bIsDeath() { return bDeath; }
	FVector GetHitTarget() const;
	AHAPlayerController* GetPlayerController();
	ECombatState GetCombatState() const;
	bool IsLocallyReloading();

	FORCEINLINE ULagCompensationComponent* GetLagCompensation () const { return LagCompensation; }
	FORCEINLINE UCombatComponent* GetCombat () const { return Combat; }
};
