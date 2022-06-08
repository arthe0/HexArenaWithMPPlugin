// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"


class UProgressBar;
class UTextBlock;
class UImage;

UCLASS()
class HEXARENA_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
public:
	/**
	*	Health and shields 
	*/

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;

	/**
	*	Timer and stats
	*/

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimerText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatsKills;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatsDeaths;

	/**
	*	Combat
	*/

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmo;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoOfType;

	UPROPERTY(meta = (BindWidget))
	UImage* WeaponImage;

	UPROPERTY(meta = (BindWidget))
	UImage* AmmoTypeImage;

	/**
	*	Net
	*/

	UPROPERTY(meta = (BindWidget))
	UImage* HighPingImage;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HighPingAnimation;

	/**
	*	Teams and gamemode
	*/

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TargetScoreText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* YellowTeamScoreBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* YellowTeamScoreText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* GreenTeamScoreBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GreenTeamScoreText;

};
