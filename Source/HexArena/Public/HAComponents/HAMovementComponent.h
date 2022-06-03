// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CombatComponent.h"
#include "HAMovementComponent.generated.h"


class ABaseWeapon;
class AHABaseCharacter;

UCLASS()
class HEXARENA_API UHAMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:

	UHAMovementComponent();

	UPROPERTY(EditAnywhere)
	float RunMultiplyer = 2.f;

	UPROPERTY(EditAnywhere)
	float AimSpeedMultiplyer = .3f;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float BaseCrouchWalkSpeed;

	UFUNCTION()
	void Run();

	UFUNCTION()
	void EndRun();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	AHABaseCharacter* HAOwnerCharacter;

	UFUNCTION()
	void OnWeaponChanged(ABaseWeapon* Weapon);
};
