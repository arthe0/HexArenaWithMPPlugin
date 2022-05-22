// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HAPlayerController.generated.h"


class AHAHUD;
class UTextBlock;

UCLASS()
class HEXARENA_API AHAPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealth (float Health, float MaxHealth);
	void SetHUDKills (float Kills);
	void SetHUDDeaths (int32 Deaths);
	void SetHUDWeaponAmmo (int32 Ammo);
	void SetHUDAmmoOfType (int32 Ammo);
	virtual void OnPossess(APawn* InPawn) override;

protected:
	virtual void BeginPlay() override;

private:
	AHAHUD* HAHUD;

	void SetNumericValueInTextBlock(float Value, UTextBlock* TextBlock);
};
