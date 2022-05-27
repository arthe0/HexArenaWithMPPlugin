// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Weapon/HitBoxTypes.h"
#include "HitBoxComponent.generated.h"

/**
 * 
 */
UCLASS()
class HEXARENA_API UHitBoxComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EHitBoxType HitBoxType = EHitBoxType::EBHT_NoHit;
};
