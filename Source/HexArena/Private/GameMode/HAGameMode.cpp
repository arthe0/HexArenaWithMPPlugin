// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/HAGameMode.h"
#include "Character/HABaseCharacter.h"
#include "PlayerController/HAPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "PlayerStates//HaPlayerState.h"

void AHAGameMode::PlayerEliminated(class AHABaseCharacter* Eliminated, AHAPlayerController* EliminatedPC, AHAPlayerController* AttackerPC)
{
	AHaPlayerState* AttackerPlayerState = AttackerPC ? Cast<AHaPlayerState>(AttackerPC->PlayerState) : nullptr;
	AHaPlayerState* EliminatedPlayerState = EliminatedPC ? Cast<AHaPlayerState>(EliminatedPC->PlayerState) : nullptr;

	if(AttackerPlayerState && AttackerPlayerState != EliminatedPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}

	if(EliminatedPlayerState)
	{
		EliminatedPlayerState->AddToDeaths(1);
	}

	if(Eliminated)
	{
		Eliminated->Death();
	}
}

void AHAGameMode::RequestRespawn(class AHABaseCharacter* Eliminated, AController* EliminatedPC)
{
	if(Eliminated)
	{
		Eliminated->Reset();
		Eliminated->Destroy();
	}
	if(EliminatedPC)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 RandomSelect = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(EliminatedPC, PlayerStarts[RandomSelect]);
	}
}
