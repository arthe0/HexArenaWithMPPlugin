// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Interactable.h"
#include "Engine/DataTable.h"
#include "LootBox.generated.h"

class UStaticMeshComponent;
class ABasePickup;

USTRUCT(BlueprintType)
struct FLootData : public FTableRowBase
{

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Loot Data")
	TSubclassOf<ABasePickup> LootClass;

	UPROPERTY(EditAnywhere, Category = "Loot Data")
	EPickupTypes PickupType;

	UPROPERTY(EditAnywhere, Category = "Loot Data")
	FName LootName;
};

UCLASS()
class HEXARENA_API ALootBox : public AInteractable
{
	GENERATED_BODY()

	ALootBox();

public:
	void OpenBox();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UFUNCTION(Category = "SpawnLoot")
	void GenerateLoot();

	UFUNCTION(Server, Reliable)
	void ServerOpenBox();

	UFUNCTION(Server, Reliable)
	void ServerGenerateLoot();

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMeshComponent* LootBoxComponent;

	UPROPERTY(EditAnywhere, Category = "LootParams")
	int32 MaxLootCount = 3;

	TArray<FLootData*> Loot;
	UDataTable* LootTable;

	UPROPERTY(EditAnywhere, Category = "LootParams")
	float RefilTime = 30.f;

	float RefilTimer = 0.f;
	bool bIsRefilling = false;

	//UFUNCTION(Server, Reliable)
	void CreateLootItems ();
};
