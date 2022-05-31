// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "HAGameMode.generated.h"

namespace MatchState
{
	extern HEXARENA_API const FName Cooldown; // Display winner and stats. Goes after match
}

class AHABaseCharacter;
class AHAPlayerController;
class AHexBlock;

UCLASS()
class HEXARENA_API AHAGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AHAGameMode();

	virtual void Tick (float DeltaTime) override;

	virtual void PlayerEliminated(class AHABaseCharacter* Eliminated, AHAPlayerController* EliminatedPC, AHAPlayerController* AttackerPC);
	virtual void RequestRespawn(class AHABaseCharacter* Eliminated, AController* EliminatedPC);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float RoundTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;

	UPROPERTY(EditDefaultsOnly)
	float EventFrequency = 20.f;

	UPROPERTY(EditDefaultsOnly)
	float EventProbability = .25f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

	virtual void MoveEvent();

private:
	float CountdownTime = 0.f;

	float MoveEventTimer = 0.f;

	int32 MaxBlockGroup = 0;
	int32 MinBlockGroup = 0;
	TMap<int32, TArray<AHexBlock*>> BlockGroups;
};
