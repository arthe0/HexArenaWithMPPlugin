// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"


class UProgressBar;
class UTextBlock;

UCLASS()
class HEXARENA_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatsKills;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatsDeaths;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmo;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoOfType;


};
