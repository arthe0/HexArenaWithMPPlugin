// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include <HATypes/Team.h>
#include "HAGameState.generated.h"

class AHaPlayerState;
class AHAGameMode;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnTeamWin, ETeam, WinTeam);

UCLASS()
class HEXARENA_API AHAGameState : public AGameState
{
	GENERATED_BODY()
	
public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(ReplicatedUsing = OnRep_TargetScore)
	int32 TargetScore = 100;

	UFUNCTION()
	void OnRep_TargetScore();
	/**
	* Teams 
	*/

	void YellowTeamScores();
	void GreenTeamScores();

	TArray<AHaPlayerState*> YellowTeam;
	TArray<AHaPlayerState*> GreenTeam;
	
	UFUNCTION()
	void OnRep_YellowTeamScore();

	UPROPERTY(ReplicatedUsing = OnRep_YellowTeamScore)
	float YellowTeamScore = 0.f;

	UFUNCTION()
	void OnRep_GreenTeamScore();

	UPROPERTY(ReplicatedUsing = OnRep_GreenTeamScore)
	float GreenTeamScore = 0.f;

	FOnTeamWin OnTeamWin;

	ETeam WinningTeam = ETeam::ET_NoTeam;

private:

public:
	void SetTargetScore (int32 NewTargetScore);

};
