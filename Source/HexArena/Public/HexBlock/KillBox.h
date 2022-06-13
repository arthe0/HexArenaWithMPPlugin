// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KillBox.generated.h"

class UBoxComponent;

UCLASS()
class HEXARENA_API AKillBox : public AActor
{
	GENERATED_BODY()
	
public:	
	AKillBox();

	UPROPERTY(EditAnywhere)
	UBoxComponent* KillBoxComponent;

	UFUNCTION()
		virtual void OnKillBoxOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult
		);

protected:
	virtual void BeginPlay() override;

public:	


};
