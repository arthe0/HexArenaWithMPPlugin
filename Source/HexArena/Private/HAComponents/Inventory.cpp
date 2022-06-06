// Fill out your copyright notice in the Description page of Project Settings.


#include "HAComponents/Inventory.h"
#include "Character/HABaseCharacter.h"
#include "HAComponents/CombatComponent.h"
#include "PlayerController/HAPlayerController.h"
#include "Weapon/BaseWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"

UInventory::UInventory()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventory::BeginPlay()
{
	Super::BeginPlay();

	if(Character->GetCombat())
	{
		Combat = Character->GetCombat();
	}
	if (Character->HasAuthority())
	{
		InitializeCarriedAmmo();
	}
}

void UInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventory, PrimaryWeapon);
	DOREPLIFETIME(UInventory, SecondaryWeapon);

	DOREPLIFETIME_CONDITION(UInventory, CurrentLightAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventory, CurrentShotgunAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventory, CurrentRifleAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventory, CurrentSniperAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventory, CurrentLauncherAmmo, COND_OwnerOnly);
}

void UInventory::InitializeCarriedAmmo()
{
	CurrentLightAmmo = StartingLightAmmo;
	CurrentShotgunAmmo = StartingShotgunAmmo;
	CurrentRifleAmmo = StartingRifleAmmo;
	CurrentSniperAmmo = StartingSniperAmmo;
	CurrentLauncherAmmo = StartingLauncherAmmo;
}

/**
* Interacting with weapon
*/

void UInventory::PickupWeapon(ABaseWeapon* WeaponToPickip)
{
	if (Character == nullptr || WeaponToPickip == nullptr) return;

	if(PrimaryWeapon == nullptr)
	{
		SetPrimaryWeapon(WeaponToPickip);
	}
	else if(SecondaryWeapon == nullptr)
	{
		SetSecondaryWeapon(WeaponToPickip);
	}
	else
	{
		if(PrimaryWeapon->GetWeaonState() == EWeaponState::EWS_Equipped)
		{
			DropPrimaryWeapon();
			SetPrimaryWeapon(WeaponToPickip);
		}
		/*else if(InventoryData.SecondaryWeapon->GetWeaonState() == EWeaponState::EWS_Equipped)
		{
			InventoryData.SecondaryWeapon->Dropped();
			InventoryData.SecondaryWeapon = WeaponToPickip;
			if (!Combat) return;
			Combat->SetWeapon(InventoryData.SecondaryWeapon);
		}*/
	}
}

void UInventory::SetPrimaryWeapon(ABaseWeapon* WeaponToSet)
{
	if (Character == nullptr || !Character->HasAuthority()) return;
	if(WeaponToSet == nullptr)
	{	
		Combat->CarriedAmmo = 0;
		PrimaryWeapon = nullptr;
		Combat->SetWeapon(PrimaryWeapon);

		if (Controller)
		{
			Controller->SetHUDAmmoOfType(0);
			Controller->SetHUDWeaponAmmo(0);
		}
		return;
	}

	PrimaryWeapon = WeaponToSet;
	PrimaryWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	Combat->SetWeapon(PrimaryWeapon);
	AttachToRightHandSocket(PrimaryWeapon);
	PrimaryWeapon->SetHUDAmmo();
	PrimaryWeapon->SetOwner(Character);

	Combat->CarriedAmmo = GetEquippedWeaponCarriedAmmo();

	//if (InventoryData.CarriedAmmoMap.Contains(WeaponToEquip->GetWeaponAmmoType()))
	//{
	//	CarriedAmmo = InventoryData.CarriedAmmoMap[WeaponToEquip->GetWeaponAmmoType()];
	//}
	SetHUDAmmo(PrimaryWeapon->GetAmmo(), GetEquippedWeaponCarriedAmmo());
}

void UInventory::OnRep_PrimaryWeapon()
{
	if (Character)
	{
		if (PrimaryWeapon == nullptr)
		{	
			SetHUDAmmo();
			return;
		}

		PrimaryWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachToRightHandSocket(PrimaryWeapon);
		if(Combat)
		{
			Combat->SetWeapon(PrimaryWeapon);
		}

		// TODO: Make update ammo in inventory rather in combat component
		SetHUDAmmo(PrimaryWeapon->GetAmmo(), GetEquippedWeaponCarriedAmmo());
	}
}

void UInventory::SetSecondaryWeapon(ABaseWeapon* WeaponToSet)
{
	if (Character == nullptr || !Character->HasAuthority()) return;
	SecondaryWeapon = WeaponToSet;
	if(!WeaponToSet) return;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_Inventory);
	AttachToSecondaryWeaponSocket(WeaponToSet);
	SecondaryWeapon->SetOwner(Character);
}

void UInventory::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_Inventory);
		AttachToSecondaryWeaponSocket(SecondaryWeapon);
		SecondaryWeapon->SetOwner(Character);
	}
}

void UInventory::SwapWeapon()
{
	if(PrimaryWeapon == nullptr && SecondaryWeapon == nullptr) return;
	ABaseWeapon* TempWeapon = PrimaryWeapon;

	SetPrimaryWeapon(SecondaryWeapon);
	SetSecondaryWeapon(TempWeapon);
	bWeaponLowered = false;
}

void UInventory::LowerWeapon()
{
	if (PrimaryWeapon == nullptr) return;
	if(bWeaponLowered)
	{
		SetPrimaryWeapon(PrimaryWeapon);
		bWeaponLowered = false;
	}
	else
	{
		PrimaryWeapon->SetWeaponState(EWeaponState::EWS_Inventory);
		Combat->SetWeapon(nullptr);
		AttachToPrimaryWeaponSocket(PrimaryWeapon);
		bWeaponLowered = true;
	}
}

/**
* Drop Weapon and Ammo
*/

void UInventory::DropPrimaryWeapon()
{
	if(!PrimaryWeapon) return;
	//PrimaryWeapon->Dropped();
	//PrimaryWeapon = nullptr;
	//Combat->SetWeapon(nullptr);
	ServerDropPrimaryWeapon();
}

void UInventory::ServerDropPrimaryWeapon_Implementation()
{
	PrimaryWeapon->Dropped();
	PrimaryWeapon = nullptr;
	Combat->SetWeapon(nullptr);
}

void UInventory::DropSecondaryWeapon()
{
	if (!SecondaryWeapon) return;
	//SecondaryWeapon->Dropped();
	//SecondaryWeapon = nullptr;
	ServerDropSecondaryWeapon();
	
}

void UInventory::ServerDropSecondaryWeapon_Implementation()
{
	SecondaryWeapon->Dropped();
	SecondaryWeapon = nullptr;
}

void UInventory::OnDeath()
{
	DropPrimaryWeapon();
	DropSecondaryWeapon();
}

/**
* Reload and ammo UPD
*/

void UInventory::UpdateAmmoValue(EAmmoType AmmoType, int32 AmountToChange)
{
	if (Character == nullptr || !Character->HasAuthority()) return;
	
	switch (AmmoType)
	{
		case EAmmoType::EAT_Light:
			CurrentLightAmmo = FMath::Clamp(CurrentLightAmmo + AmountToChange, 0, MaxLightAmmo);
		break;

		case EAmmoType::EAT_Shotgun:
			CurrentShotgunAmmo = FMath::Clamp(CurrentShotgunAmmo + AmountToChange, 0, MaxShotgunAmmo);
		break;

		case EAmmoType::EAT_Rifle:
			CurrentRifleAmmo = FMath::Clamp(CurrentRifleAmmo + AmountToChange, 0, MaxRifleAmmo);
		break;

		case EAmmoType::EAT_Sniper:
			CurrentSniperAmmo = FMath::Clamp(CurrentSniperAmmo + AmountToChange, 0, MaxSniperAmmo);
		break;

		case EAmmoType::EAT_Launcher:
			CurrentLauncherAmmo = FMath::Clamp(CurrentLauncherAmmo + AmountToChange, 0, MaxLauncherAmmo);
		break;
	}
}

void UInventory::Reload()
{
	if(PrimaryWeapon)
	{
		int32 ToReload = AmountToReload();
		UE_LOG(LogTemp, Warning, TEXT("ToReload = % d"), ToReload);
		PrimaryWeapon->AddAmmo(ToReload);
		UpdateAmmoValue(PrimaryWeapon->WeaponData.AmmoType, -ToReload);
		Controller->SetHUDAmmoOfType(GetEquippedWeaponCarriedAmmo());
		Combat->CarriedAmmo = GetEquippedWeaponCarriedAmmo();
	}
}

int32 UInventory::AmountToReload()
{
	if (PrimaryWeapon == nullptr) return 0;
	int32 SpaceInMag = PrimaryWeapon->GetMagCapacity() - PrimaryWeapon->GetAmmo();

	int32 Least = FMath::Min(SpaceInMag, GetEquippedWeaponCarriedAmmo());
	return FMath::Clamp(SpaceInMag, 0, Least);
}

int32 UInventory::GetEquippedWeaponCarriedAmmo()
{
	if(!PrimaryWeapon) return 0;
	switch (PrimaryWeapon->WeaponData.AmmoType)
	{
	case EAmmoType::EAT_Light:
		return CurrentLightAmmo;
		break;

	case EAmmoType::EAT_Shotgun:
		return CurrentShotgunAmmo;
		break;

	case EAmmoType::EAT_Rifle:
		return CurrentRifleAmmo;
		break;

	case EAmmoType::EAT_Sniper:
		return CurrentSniperAmmo;
		break;

	case EAmmoType::EAT_Launcher:
		return CurrentLauncherAmmo;
		break;
	}

	return 0;
}

void UInventory::SetHUDAmmo(int32 WeaponAmmo /*= 0*/, int32 CarriedAmmo /*= 0*/)
{
	Controller = Controller == nullptr ? Cast <AHAPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDAmmoOfType(CarriedAmmo);
		Controller->SetHUDWeaponAmmo(WeaponAmmo);
	}
}

/**
* Socket Attachment
*/

void UInventory::AttachToPrimaryWeaponSocket(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* PrimaryWeaponSocket = Character->GetMesh()->GetSocketByName(FName("PrimaryWeaponSocket"));
	if (PrimaryWeaponSocket)
	{
		PrimaryWeaponSocket->AttachActor(PrimaryWeapon, Character->GetMesh());
	}
}

void UInventory::AttachToSecondaryWeaponSocket(AActor* ActorToAttach)
{
	if(Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* SecondaryWeaponSocket = Character->GetMesh()->GetSocketByName(FName("SecondaryWeaponSocket"));
	if (SecondaryWeaponSocket)
	{
		SecondaryWeaponSocket->AttachActor(SecondaryWeapon, Character->GetMesh());
	}
}

void UInventory::AttachToRightHandSocket(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (RightHandSocket)
	{
		RightHandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

/**
* OnReps for updating Clients HUD
*/

void UInventory::OnRep_LightAmmo()
{
	if(PrimaryWeapon && Combat && PrimaryWeapon->WeaponData.AmmoType == EAmmoType::EAT_Light)
	{
		Combat->CarriedAmmo = CurrentLightAmmo;
	}
}

void UInventory::OnRep_RifleAmmo()
{
	if (PrimaryWeapon && Combat && PrimaryWeapon->WeaponData.AmmoType == EAmmoType::EAT_Rifle)
	{
		Combat->CarriedAmmo = CurrentRifleAmmo;
	}
}

void UInventory::OnRep_ShotgunAmmo()
{
	if (PrimaryWeapon && Combat && PrimaryWeapon->WeaponData.AmmoType == EAmmoType::EAT_Shotgun)
	{
		Combat->CarriedAmmo = CurrentShotgunAmmo;
	}
}

void UInventory::OnRep_SniperAmmo()
{
	if (PrimaryWeapon && Combat && PrimaryWeapon->WeaponData.AmmoType == EAmmoType::EAT_Sniper)
	{
		Combat->CarriedAmmo = CurrentSniperAmmo;
	}
}

void UInventory::OnRep_LauncherAmmo()
{
	if (PrimaryWeapon && Combat && PrimaryWeapon->WeaponData.AmmoType == EAmmoType::EAT_Launcher)
	{
		Combat->CarriedAmmo = CurrentLauncherAmmo;
	}
}

