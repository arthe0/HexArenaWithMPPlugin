// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "HexBlock.generated.h"

class UStaticMeshComponent;
class UCurveFloat;

UCLASS()
class HEXARENA_API AHexBlock : public AActor
{
	GENERATED_BODY()
	
public:	
	AHexBlock();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void RiseBlock();

	UFUNCTION(BlueprintCallable)
	void LowerBlock();

	UFUNCTION(BlueprintCallable)
	void StartPosition();

protected:

private:

	virtual void BeginPlay() override;

	UFUNCTION()
	void TimelineProgress(float Value);

	float BlockHeight = 0.f;

	UPROPERTY(EditAnywhere, Category = "Move Curve")
	float MovingMultiplyer = 100.f;

	//UPROPERTY(EditAnywhere, Category = "Move Curve")
	//float LoweMultiplyer = 100.f;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMeshComponent* HexMeshComponent;

	FTimeline RiseTimeline;
	
	//FTimeline LowerTimeline;

	UPROPERTY(EditAnywhere, Category = "Move Curve")
	UCurveFloat* RiseCurve;

	//UPROPERTY(EditAnywhere, Category = "Move Curve")
	//UCurveFloat* LowerCurve;

	UPROPERTY()
	FVector DefaultLocation;

	UPROPERTY()
	FVector RiseLocation;

	UPROPERTY()
	FVector LowerLocation;

	UPROPERTY()
	FVector CurrentLocation;

	UPROPERTY()
	FVector MoveToLocation;





public:	

};
