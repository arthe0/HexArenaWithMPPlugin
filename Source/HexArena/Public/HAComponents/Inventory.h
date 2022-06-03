// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapon/AmmoTypes.h"
#include "Inventory.generated.h"

class ABaseWeapon;
class AHABaseCharacter;

USTRUCT(BlueprintType)
struct FInventoryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	ABaseWeapon* PrimaryWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	ABaseWeapon* SecondaryWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	TMap<EAmmoType, int32> CarriedAmmoMap;

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

	void InitializeCarriedAmmo()
	{
		CarriedAmmoMap.Emplace(EAmmoType::EAT_Light, StartingLightAmmo);
		CarriedAmmoMap.Emplace(EAmmoType::EAT_Shotgun, StartingShotgunAmmo);
		CarriedAmmoMap.Emplace(EAmmoType::EAT_Rifle, StartingRifleAmmo);
		CarriedAmmoMap.Emplace(EAmmoType::EAT_Sniper, StartingSniperAmmo);
		CarriedAmmoMap.Emplace(EAmmoType::EAT_Launcher, StartingLauncherAmmo);
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HEXARENA_API UInventory : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventory();

	friend AHABaseCharacter;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	FInventoryData InventoryData;
	
protected:

	virtual void BeginPlay() override;

private:
	AHABaseCharacter* Character;

public:	
		
};
