// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
//#include "HATypes/Team.h"
#include "HaPlayerState.generated.h"

class AHABaseCharacter;
class AHAPlayerController;

UCLASS()
class HEXARENA_API AHaPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	/**
	* Replication notifies
	*/
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Defeats();

	void AddToScore(float ScoreAmount);
	void AddToDeaths(int32 DeathsAmount);
private:
	UPROPERTY()
	class AHABaseCharacter* Character;

	UPROPERTY()
	class AHAPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats = 0;

	UPROPERTY(Replicated)
	float Kills = 0.f;

	//UFUNCTION()
	//void OnRep_Team();

	
public:
	
};
