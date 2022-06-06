// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "HUD/HAHUD.h"
#include "Weapon/AmmoTypes.h"
#include "HATypes/CombatState.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangeWeaponDelegate, ABaseWeapon*, EquippedWeapon);

class ABaseWeapon;
class AHABaseCharacter;
class AHAPlayerController;
class AHAHUD;
class UCurveFloat;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HEXARENA_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	friend AHABaseCharacter;
	UCombatComponent();


	// Custom Delegate to let know other classes which weapon character holding now
	FOnChangeWeaponDelegate OnChangeWeaponDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aiming")
	UCurveFloat* AimingCurve;

	FTimeline AimingTimeline;

	bool bLocalyReloading = false;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void Reload();

	// Carried for a Weapon ammo type uses for HUD and reload starting
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;


protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void SetAiming(bool bIsAiming);
	void CalculateHipSpread(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	void FireButtonPressed(bool bPressed);
	void Fire();

	void LocalFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION()
	void TimelineProgress(const float Value);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);
	void SetHUDCrosshairs();
	void InterpFOV(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 AmountToReload();
	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	
private:


	AHABaseCharacter* Character;

	AHAPlayerController* Controller; 

	AHAHUD* HUD;

	UPROPERTY(Replicated/*Using = OnRep_EquippedWeapon*/)
	ABaseWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;

	bool bLocalAiming = false;

	UFUNCTION()
	void OnRep_Aiming();

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float BaseWalkCrouchSpeed;

	UPROPERTY(EditAnywhere)
	float RunMultiplyer;

	UPROPERTY(EditAnywhere)
	float AimMultiplyer;

	bool bFireButtonPressed;

	/*
	* HUD and crosshairs
	*/
	float HipSpread = 0.f;

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairInAimFactor;
	float CrosshairShootingFactor;
	FVector HitTarget;

	FHUDPackage HUDPackage;

	/*
	* Aiming and FOV
	*/

	// Field of view when not aiming
	float DefaultFOV;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, /*Replicated,*/ Category = Anim)
	float ADSWeight = 0.f;

	/*
	* Auto Fire
	*/

	FTimerHandle FireTimer;

	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();
	
	bool CanFire();

	/*
	* Ammo
	*/


	UFUNCTION()
	void OnRep_CarriedAmmo();

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingLightAmmo = 30;
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingShotgunAmmo = 5;
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingRifleAmmo = 30;
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingSniperAmmo = 5;
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingLauncherAmmo = 1;

	TMap<EAmmoType, int32> CarriedAmmoMap;

	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupide;

	UFUNCTION()
	void OnRep_CombatState();
public:	
	void SetWeapon(ABaseWeapon* WeaponToEquip);	
};
