// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "HAGameMode.generated.h"


class AHABaseCharacter;
class AHAPlayerController;

UCLASS()
class HEXARENA_API AHAGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(class AHABaseCharacter* Eliminated, AHAPlayerController* EliminatedPC, AHAPlayerController* AttackerPC);
	virtual void RequestRespawn(class AHABaseCharacter* Eliminated, AController* EliminatedPC);
};
