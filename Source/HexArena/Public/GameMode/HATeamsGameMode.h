// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/HAGameMode.h"
#include "HATypes/Team.h"
#include "HATeamsGameMode.generated.h"


UCLASS()
class HEXARENA_API AHATeamsGameMode : public AHAGameMode
{
	GENERATED_BODY()

public:
	AHATeamsGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float Damage) override;
	virtual void PlayerEliminated(class AHABaseCharacter* Eliminated, AHAPlayerController* EliminatedPC, AHAPlayerController* AttackerPC) override;
	
	UFUNCTION()
	void OnTeamWin(ETeam Team);
protected:
	virtual void HandleMatchHasStarted() override;


	
};
