// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/HATeamsGameMode.h"
#include "GameState/HAGameState.h"
#include "PlayerStates/HaPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include <PlayerController/HAPlayerController.h>

AHATeamsGameMode::AHATeamsGameMode()
{
	bTeamsMath = true;
}

void AHATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AHAGameState* HAGameState = Cast<AHAGameState>(UGameplayStatics::GetGameState(this));
	if (HAGameState)
	{
		AHaPlayerState* HAPState = NewPlayer->GetPlayerState<AHaPlayerState>();
		if (HAPState && HAPState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (HAGameState->GreenTeam.Num() >= HAGameState->YellowTeam.Num())
			{
				HAGameState->YellowTeam.AddUnique(HAPState);
				HAPState->SetTeam(ETeam::ET_YellowTeam);
			}
			else
			{
				HAGameState->GreenTeam.AddUnique(HAPState);
				HAPState->SetTeam(ETeam::ET_GreenTeam);
			}
		}	
	}
}

void AHATeamsGameMode::Logout(AController* Exiting)
{
	AHAGameState* HAGameState = Cast<AHAGameState>(UGameplayStatics::GetGameState(this));
	AHaPlayerState* HAPState = Exiting->GetPlayerState<AHaPlayerState>();

	if(HAGameState && HAPState)
	{
		if(HAGameState->YellowTeam.Contains(HAPState))
		{
			HAGameState->YellowTeam.Remove(HAPState);
		}
		else if(HAGameState->GreenTeam.Contains(HAPState))
		{
			HAGameState->GreenTeam.Remove(HAPState);
		}
	}
}

void AHATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AHAGameState* HAGameState = Cast<AHAGameState>(UGameplayStatics::GetGameState(this));
	if(HAGameState)
	{
		HAGameState->OnTeamWin.BindUFunction(this, FName("OnTeamWin"));

		for(auto PlayerState : HAGameState->PlayerArray)
		{
			AHaPlayerState* HAPState = Cast<AHaPlayerState>(PlayerState.Get());
			if(HAPState && HAPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if(HAGameState->GreenTeam.Num() >= HAGameState->YellowTeam.Num())
				{
					HAGameState->YellowTeam.AddUnique(HAPState);
					HAPState->SetTeam(ETeam::ET_YellowTeam);
				}
				else
				{
					HAGameState->GreenTeam.AddUnique(HAPState);
					HAPState->SetTeam(ETeam::ET_GreenTeam);
				}
			}
		}
	}
}

float AHATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float Damage)
{
	AHaPlayerState* AttackerPS = Attacker->GetPlayerState<AHaPlayerState>();
	AHaPlayerState* VictimPS = Victim->GetPlayerState<AHaPlayerState>();

	if(!AttackerPS || !VictimPS) return Damage;
	if(VictimPS == AttackerPS) return Damage;

	if(AttackerPS->GetTeam() == VictimPS->GetTeam()) return 0.f;

	return Damage;
}

void AHATeamsGameMode::PlayerEliminated(AHABaseCharacter* Eliminated, AHAPlayerController* EliminatedPC, AHAPlayerController* AttackerPC)
{
	Super::PlayerEliminated(Eliminated, EliminatedPC, AttackerPC);

	AHAGameState* HAGameState = Cast<AHAGameState>(UGameplayStatics::GetGameState(this));
	AHaPlayerState* AttackerPS = AttackerPC ? Cast<AHaPlayerState>(AttackerPC->PlayerState) : nullptr;
	if(HAGameState && AttackerPS)
	{
		if (AttackerPS->GetTeam() == ETeam::ET_GreenTeam)
		{
			HAGameState->GreenTeamScores();
		}
		if(AttackerPS->GetTeam() == ETeam::ET_YellowTeam)
		{
			HAGameState->YellowTeamScores();
		}
	}
}

void AHATeamsGameMode::OnTeamWin(ETeam Team)
{
	RoundTime = GetWorld()->GetTimeSeconds() - LevelStartingTime;
	SetMatchState(MatchState::Cooldown);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AHAPlayerController* Player = Cast<AHAPlayerController>(*It);
		if (Player)
		{
			Player->SetRoundTime(RoundTime);
		}
	}
}

