// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class HEXARENA_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	UPROPERTY(EditDefaultsOnly)
	int32 NumPlayersToStart = 10;

	UPROPERTY(EditDefaultsOnly)
	FString LevelToTravelPath = FString("/Game/Levels/HALevel?listen");

	UFUNCTION(BlueprintCallable)
	void TravelToLevel(FString Path);

	UFUNCTION(BlueprintCallable)
	void StartTravel();
	
};
