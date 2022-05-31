// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "HexBlock.generated.h"

class UStaticMeshComponent;
class UCurveFloat;

UENUM(BlueprintType)
enum class EBlockState : uint8
{
	EBS_Default UMETA(DisplayName = "DefaultBS"),
	EBS_Rised UMETA(DisplayName = "RisedBS"),
	EBS_Lowered UMETA(DisplayName = "LoweredBS"),

	EHBT_MAX UMETA(DisplayName = "DefaulMAX")
};

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

	UPROPERTY(EditAnywhere)
	int32 BlockGroup = 1;

	UPROPERTY()
	EBlockState BlockState = EBlockState::EBS_Default;

protected:

private:

	virtual void BeginPlay() override;

	UFUNCTION()
	void TimelineProgress(float Value);

	UFUNCTION(Server, Reliable)
	void ServerTimelineProgress(const FVector_NetQuantize& NewLocation);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastTimelineProgress(const FVector_NetQuantize& NewLocation);

	UPROPERTY(EditAnywhere, Category = "Move Curve")
	float MovingMultiplyer = 100.f;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMeshComponent* HexMeshComponent;

	FTimeline RiseTimeline;
	
	UPROPERTY(EditAnywhere, Category = "Move Curve")
	UCurveFloat* RiseCurve;

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
