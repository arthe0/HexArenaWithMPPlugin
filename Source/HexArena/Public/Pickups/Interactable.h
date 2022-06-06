// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickups/PickupTypes.h"
#include "Interactable.generated.h"

#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252

class USphereComponent;
class UWidgetComponent;

UCLASS()
class HEXARENA_API AInteractable : public AActor
{
	GENERATED_BODY()
	
public:	
	AInteractable();

	void ShowPickupWidget(bool bShowWidget);

	UPROPERTY(EditAnywhere)
	EPickupTypes PickupType;

	UPROPERTY(EditAnywhere)
	bool bAutoPickup = false;

protected:
	virtual void BeginPlay() override;

	virtual void EnableCustomDepth(bool bEnable) {};

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UWidgetComponent* PickupWidget;

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

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* AreaSphere;

public:	


};
