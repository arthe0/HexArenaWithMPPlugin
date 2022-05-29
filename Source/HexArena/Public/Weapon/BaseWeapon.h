// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/AmmoTypes.h"
#include "Weapon/WeaponTypes.h"
#include "Engine/DataTable.h"
#include "BaseWeapon.generated.h"

class AProjectile;
class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;
class UTexture2D;
class AHABaseCharacter;
class AHAPlayerController;

//Struct for FPP 
USTRUCT(BlueprintType)
struct FIKProperties
{
	class UAnimSequence;

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimSequence* AnimPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AimOffset = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform CustomOffsetTranform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LeftHandModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector RightHandModifier;
};

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX"),
};

//Struct for WeaponData
USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	class UAnimSequence;
	class USkeletalMeshComponent;

	GENERATED_BODY()

	/*
	* Table Data
	*/

	UPROPERTY(EditAnywhere, Category = "Table Data")
	FName WeaponName;

	/*
	* Weapon Mesh
	*/

	UPROPERTY(EditAnywhere, Category = "Weapon SK Mesh")
	USkeletalMesh* WeaponMesh;

	/*
	 *  Effects and animations
	 */

	UPROPERTY(EditAnywhere, Category = "Effects and animations")
	UAnimationAsset* FireAnimation;

	/*
	* Ammo and Damage
	*/

	UPROPERTY(EditAnywhere, Category = "Ammo")
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 MagCapacity = 30;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	TSubclassOf<class ABulletShell> BulletShellClass;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	EAmmoType AmmoType = EAmmoType::EAT_Rifle;

	UPROPERTY(EditAnywhere, Category = "AmmoDamage")
	float BaseDamage = 18.f;

	UPROPERTY(EditAnywhere, Category = "AmmoDamage")
	float HeadMultiplyer = 2.f;

	UPROPERTY(EditAnywhere, Category = "AmmoDamage")
	float NeckMultiplyer = 1.8f;

	UPROPERTY(EditAnywhere, Category = "AmmoDamage")
	float ChestMultiplyer = 1.f;

	UPROPERTY(EditAnywhere, Category = "AmmoDamage")
	float StomachMultiplyer = 0.9f;

	UPROPERTY(EditAnywhere, Category = "AmmoDamage")
	float LimbsMultiplyer = .8f;

	/*
	* Weapon Type
	*/

	UPROPERTY(EditAnywhere, Category = "Weapon Type")
	EWeaponType WeaponType = EWeaponType::EWT_Rifle;

	/*
	* Automatic
	*/

	UPROPERTY(EditAnywhere, Category = "Automatic")
	float FireRate = 600.f;

	UPROPERTY(EditAnywhere, Category = "Automatic")
	bool bSingle = true;

	UPROPERTY(EditAnywhere, Category = "Automatic")
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere, Category = "Automatic")
	bool bBurst = true;

	UPROPERTY(EditAnywhere, Category = "Automatic")
	int32 ShotsInBurst = 3;

	/*
	* Aim properties
	*/
	UPROPERTY(EditAnywhere, Category = "Aim")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Aim")
	float ZoomInterpSpeed = 20.f;

	/*
	* IKTransformProperties
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
	FIKProperties IKProperties;

	/*
	* Crosshair
	*/

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairsBottom;
};



UCLASS()
class HEXARENA_API ABaseWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseWeapon();

	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	void ShowPickupWidget(bool bShowWidget);
	void SetHUDAmmo();
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	void AddAmmo(int32 AmmoToAdd);

	/*
	* WeaponData
	*/

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "WeaponData")
	FWeaponData WeaponData;

	 /*
	 * Automatic
	 */

	 float FireDelay;

protected:
	virtual void BeginPlay() override;

	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnDropped();

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	UFUNCTION()
	void OnPingToHigh(bool bPingTooHigh);

	/**
	* SSR
	*/
	UPROPERTY(Replicated, EditAnywhere)
	bool bUseSSR = true;


private:	

	UPROPERTY(EditAnywhere, Category = "Weapon Mesh")
	USkeletalMeshComponent* WeaponMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UWidgetComponent* PickupWidget;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere/*, ReplicatedUsing = OnRep_Ammo*/)
	int32 Ammo;

	UPROPERTY(EditAnywhere, Category = "Table Data")
	FName WeaponName;

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	void SpendRound();

	//	The number of unprocessed server requses for ammo.
	//	Incremented in SpendRound, Decremented in FireUpdateAmmo
	int32 UnprocessedSequence = 0;

	UPROPERTY()
	AHABaseCharacter* HAOwnerCharacter;

	UPROPERTY()
	AHAPlayerController* HAOwnerController;

	/*
	* Ammo
	*/

public:
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() { return WeaponMeshComponent; }
	FORCEINLINE float GetZoomedFOV() const { return WeaponData.ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return WeaponData.ZoomInterpSpeed; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return WeaponData.MagCapacity; }
	bool IsEmpty();
	FORCEINLINE EAmmoType GetWeaponAmmoType() const { return WeaponData.AmmoType; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponData.WeaponType; }
	virtual FORCEINLINE FTransform GetsightsWorldTransform_Implementation() { return WeaponMeshComponent->GetSocketTransform(FName("Sights")); }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "IK")
	FTransform GetsightsWorldTransform() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "WeaponData")
	FName GetWeaponName();
};
