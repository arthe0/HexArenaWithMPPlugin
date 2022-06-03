// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupTypes.h"
#include "BasePickup.generated.h"

class USphereComponent;
class UWidgetComponent;

UCLASS()
class HEXARENA_API ABasePickup : public AActor
{
	GENERATED_BODY()
	
public:	
	ABasePickup();

	void ShowPickupWidget(bool bShowWidget);

	UPROPERTY(EditAnywhere)
	EPickupTypes PickupType;

protected:
	virtual void BeginPlay() override;

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

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UWidgetComponent* PickupWidget;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
