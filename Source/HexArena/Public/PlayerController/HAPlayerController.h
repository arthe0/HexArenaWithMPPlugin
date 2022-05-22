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
	virtual void Tick(float DeltaTime) override;

	void SetHUDHealth (float Health, float MaxHealth);
	void SetHUDKills (float Kills);
	void SetHUDDeaths (int32 Deaths);
	void SetHUDWeaponAmmo (int32 Ammo);
	void SetHUDAmmoOfType (int32 Ammo);
	virtual void OnPossess(APawn* InPawn) override;

protected:
	virtual void BeginPlay() override;


	void HighPingWarning();
	void StopHighPingWarning();

	void CheckPing(float DeltaTime);

private:
	AHAHUD* HAHUD;

	void SetNumericValueInTextBlock(float Value, UTextBlock* TextBlock);


	float HighPingRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 10.f;

	float PlayAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 15.f;

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 100.f;
};
