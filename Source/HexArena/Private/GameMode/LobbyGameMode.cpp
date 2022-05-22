// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LobbyGameMode.h"
#include "GameFramework//GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NUmberOfPlayers = GameState.Get()->PlayerArray.Num();
	if(NUmberOfPlayers == NumPlayersToStart)
	{
		TravelToLevel(LevelToTravelPath);
	}
}

void ALobbyGameMode::TravelToLevel(FString Path)
{
	UWorld* World = GetWorld();
	if (World)
	{
		bUseSeamlessTravel = true;
		World->ServerTravel(Path);
	}
}

void ALobbyGameMode::StartTravel()
{
	TravelToLevel(LevelToTravelPath);
}
