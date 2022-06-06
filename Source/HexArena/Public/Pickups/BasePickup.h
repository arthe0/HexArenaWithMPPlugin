// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupTypes.h"
#include "Interactable.h"
#include "BasePickup.generated.h"


class USphereComponent;
class UWidgetComponent;class UCharacterMovementComponent;

UCLASS()
class HEXARENA_API ABasePickup : public AInteractable
{
	GENERATED_BODY()
	
public:	
	ABasePickup();

	void AddImpulse(FVector Vector, FName BoneName = NAME_None, bool bVelocity = false);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* PhysicsMeshComponent;

public:	

	FORCEINLINE UStaticMeshComponent* GetPhysicsMesh() { return PhysicsMeshComponent; }

};
