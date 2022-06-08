// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/BasePickup.h"
#include "Weapon/AmmoTypes.h"
#include "Engine/DataTable.h"
#include "AmmoPickup.generated.h"


USTRUCT(BlueprintType)
struct FAmmoPickupData : public FTableRowBase
{
	class UAnimSequence;
	class UStaticMeshComponent;

	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EAmmoType AmmoType = EAmmoType::EAT_Rifle;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMesh* Mesh;
};

UCLASS()
class HEXARENA_API AAmmoPickup : public ABasePickup
{
	GENERATED_BODY()

public:
	AAmmoPickup();

	UPROPERTY(BlueprintReadWrite)
	FAmmoPickupData AmmoPickupData;

	UPROPERTY(EditAnywhere, Category = "Table Data")
	FName AmmoName = "RifleAmmo";

	void SetAmmoDataByName(FName NewName);

protected:
	virtual void BeginPlay() override;
	virtual void EnableCustomDepth(bool bEnable) override;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* StaticMeshComponent;
private:
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetAmmoDataByName(FName NewName);

	UDataTable* AmmoTable;

	UPROPERTY(EditAnywhere)
	int32 AmmoAmount = 30;
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FName GetAmmoName();

	FORCEINLINE int32 GetAmmoAmount () { return AmmoAmount; }


};
