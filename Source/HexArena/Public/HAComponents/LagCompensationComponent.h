// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapon/HitBoxTypes.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "LagCompensationComponent.generated.h"

class AHAPlayerController;
class AHABaseCharacter;

USTRUCT(BlueprintType)
struct FBoxParams
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxParams> HitBoxParams;

	UPROPERTY()
	AHABaseCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	EHitBoxType HittedBox;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HEXARENA_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class AHABaseCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage (const FFramePackage& Package, FColor Color);

	/**
	* Projectile
	*/

	UFUNCTION(Server, Reliable)
		void ProjectileServerScoreRequest(
			AHABaseCharacter* HitCharacter,
			const FVector_NetQuantize& TraceStart,
			const FVector_NetQuantize100& Initialvelocity,
			float HitTime
		);

	FServerSideRewindResult ProjectileServerSideRewind(
		AHABaseCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& Initialvelocity,
		float HitTime
	);

protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	
	FFramePackage GetFrameToCheck(AHABaseCharacter* HitCharacter, float HitTime);

	void CacheBoxPositions(AHABaseCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(AHABaseCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(AHABaseCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(AHABaseCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	bool CheckIfBoxHitted(AHABaseCharacter* HitCharacter, FPredictProjectilePathParams& PathParams, FPredictProjectilePathResult& PathResult, FName BoxName);

	/**
	* Projectile
	*/

	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		AHABaseCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& Initialvelocity,
		float HitTime
	);

private:
	UPROPERTY()
	AHABaseCharacter* Character;

	UPROPERTY()
	AHAPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistroy;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;

public:	
	

		
};
