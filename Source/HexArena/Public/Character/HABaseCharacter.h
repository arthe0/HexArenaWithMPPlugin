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
#include "HABaseCharacter.generated.h"



class UCombatComponent;
class UHealthComponent;
class UCameraComponent;
class UWidgetComponent;
class ABaseWeapon;
class UAnimMontage;
class AHAPlayerController;
class UTimelineComponent;
class AHaPlayerState;
class UBoxComponent;
class ULagCompensationComponent;
class UBoxComponent;

UCLASS()
class HEXARENA_API AHABaseCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	AHABaseCharacter();
	
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
	TMap<FName, UBoxComponent*> HitBoxes;

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
	void AimOffset(float DeltaTime);

	//	Poll for any relevant class and init HUD
	void PollInit();

	/*
	 * Hit Boxes for server side rewind
	 */

	UPROPERTY(EditAnywhere)
	UBoxComponent* HeadBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* NeckBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* ChestBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* StomachBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* PelvisBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperArmLBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperArmRBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* LowerArmLBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* LowerArmRBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* HandLBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* HandRBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* ThighLBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* ThighRBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* CalfLBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* CalfRBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* FootLBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* FootRBox;

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USkeletalMeshComponent* ClientMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	ABaseWeapon* OverlappingWeapon;

	/*
	 *	HA Components 
	 */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere)
	UHealthComponent* Health;

	UPROPERTY(VisibleAnywhere)
	ULagCompensationComponent* LagCompensation;

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

	//UPROPERTY(Replicated)
	//float Direction;

	void PlayHitReactMontage();

	UFUNCTION()
	void OnRep_OverlappingWeapon(ABaseWeapon* LastWeapon);	

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

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
	void SetOverlappingWeapon(ABaseWeapon* Weapon);
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
};
