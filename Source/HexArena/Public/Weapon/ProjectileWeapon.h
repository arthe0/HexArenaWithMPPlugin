// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "ProjectileWeapon.generated.h"


class AProjectile;

UCLASS()
class HEXARENA_API AProjectileWeapon : public ABaseWeapon
{
	GENERATED_BODY()
	
public:
	virtual void Fire (const FVector& HitTarget) override;
private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ServerSideRewindProjectileClass;
};
