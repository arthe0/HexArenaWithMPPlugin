// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapon/AmmoTypes.h"
#include "Inventory.generated.h"

class ABaseWeapon;
class AHABaseCharacter;
class AAmmoPickup;
class UCombatComponent;
class AHAPlayerController;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HEXARENA_API UInventory : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventory();

	friend AHABaseCharacter;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UFUNCTION(BlueprintCallable)
	void PickupWeapon(ABaseWeapon* WeaponToPickip);

	UFUNCTION(BlueprintCallable)
	void SwapWeapon();

	UFUNCTION(BlueprintCallable)
	void LowerWeapon();

	void AttachToPrimaryWeaponSocket(AActor* ActorToAttach);
	void AttachToSecondaryWeaponSocket(AActor* ActorToAttach);
	void AttachToRightHandSocket(AActor* ActorToAttach);

	void Reload();
	void DropPrimaryWeapon();

	UFUNCTION(Server, Reliable)
	void ServerDropPrimaryWeapon();

	void DropSecondaryWeapon();

	UFUNCTION(Server, Reliable)
	void ServerDropSecondaryWeapon();

	UFUNCTION()
	void OnDeath();

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_PrimaryWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

private:
	AHABaseCharacter* Character;
	UCombatComponent* Combat;
	AHAPlayerController* Controller;

	// Must be equipped
	UPROPERTY(ReplicatedUsing = OnRep_PrimaryWeapon, EditAnywhere, Category = "Weapon")
	ABaseWeapon* PrimaryWeapon;

	// Must be in inventory
	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon, EditAnywhere, Category = "Weapon")
	ABaseWeapon* SecondaryWeapon;

	//TMap<EAmmoType, int32> CarriedAmmoMap;

	// Back TMap and make client hud update via server
	UPROPERTY(ReplicatedUsing = OnRep_LightAmmo)
	int32 CurrentLightAmmo = 0;
	UPROPERTY(ReplicatedUsing = OnRep_ShotgunAmmo)
	int32 CurrentShotgunAmmo = 0;
	UPROPERTY(ReplicatedUsing = OnRep_RifleAmmo)
	int32 CurrentRifleAmmo = 0;
	UPROPERTY(ReplicatedUsing = OnRep_SniperAmmo)
	int32 CurrentSniperAmmo = 0;
	UPROPERTY(ReplicatedUsing = OnRep_LauncherAmmo)
	int32 CurrentLauncherAmmo = 0;

	UFUNCTION()
	void OnRep_LightAmmo();
	UFUNCTION()
	void OnRep_RifleAmmo();
	UFUNCTION()
	void OnRep_ShotgunAmmo();
	UFUNCTION()
	void OnRep_SniperAmmo();
	UFUNCTION()
	void OnRep_LauncherAmmo();

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 MaxLightAmmo = 150;
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 MaxShotgunAmmo = 40;
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 MaxRifleAmmo = 210;
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 MaxSniperAmmo = 40;
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 MaxLauncherAmmo = 5;

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
	
	UFUNCTION()
	void InitializeCarriedAmmo();
	
	void SetPrimaryWeapon(ABaseWeapon* WeaponToSet);
	void SetSecondaryWeapon(ABaseWeapon* WeaponToSet);

	int32 GetEquippedWeaponCarriedAmmo();
	void SetHUDAmmo(int32 WeaponAmmo = 0, int32 CarriedAmmo = 0);

	void UpdateAmmoValue(EAmmoType AmmoType, int32 AmountToChange);
	int32 AmountToReload();

	bool bWeaponLowered = false;
public:	

		
};
