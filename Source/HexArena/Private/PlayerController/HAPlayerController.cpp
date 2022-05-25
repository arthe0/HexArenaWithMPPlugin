// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/HAPlayerController.h"
#include "HUD/HAHUD.h"
#include "HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Character/HABaseCharacter.h"
#include "Components/Image.h"


void AHAPlayerController::BeginPlay()
{
	Super::BeginPlay();

	HAHUD = Cast<AHAHUD>(GetHUD());
}

void AHAPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckPing(DeltaTime);
}

void AHAPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<AHaPlayerState>() : PlayerState;
		if (PlayerState)
		{
			if (PlayerState->GetCompressedPing() * 4 > HighPingThreshold) // Ping is compressed by 4 by default
			{
				HighPingWarning();
				PlayAnimationRunningTime = 0.f;
			}
		}
		HighPingRunningTime = 0.f;
	}

	bool bHighPingANimationPlaying =
		HAHUD &&
		HAHUD->CharacterOverlay &&
		HAHUD->CharacterOverlay->HighPingAnimation &&
		HAHUD->CharacterOverlay->IsAnimationPlaying(HAHUD->CharacterOverlay->HighPingAnimation);

	if (bHighPingANimationPlaying)
	{
		PlayAnimationRunningTime += DeltaTime;
		if (PlayAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}



void AHAPlayerController::HighPingWarning()
{
	HAHUD = HAHUD == nullptr ? Cast<AHAHUD>(GetHUD()) : HAHUD;

	bool bHUDValid = HAHUD &&
		HAHUD->CharacterOverlay &&
		HAHUD->CharacterOverlay->HighPingImage &&
		HAHUD->CharacterOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		HAHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		HAHUD->CharacterOverlay->PlayAnimation(HAHUD->CharacterOverlay->HighPingAnimation, 0.f, 10);
	}
}

void AHAPlayerController::StopHighPingWarning()
{
	HAHUD = HAHUD == nullptr ? Cast<AHAHUD>(GetHUD()) : HAHUD;

	bool bHUDValid = HAHUD &&
		HAHUD->CharacterOverlay &&
		HAHUD->CharacterOverlay->HighPingImage &&
		HAHUD->CharacterOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		HAHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if(HAHUD->CharacterOverlay->IsAnimationPlaying(HAHUD->CharacterOverlay->HighPingAnimation))
		{
			HAHUD->CharacterOverlay->StopAnimation(HAHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

void AHAPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AHABaseCharacter* HACharacter = Cast<AHABaseCharacter>(InPawn);
	if(HACharacter)
	{
		SetHUDHealth(HACharacter->GetHealthComponent()->GetHealth(), HACharacter->GetHealthComponent()->GetMaxHealth());
	}
}


void AHAPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	HAHUD = HAHUD == nullptr ? Cast<AHAHUD>(GetHUD()) : HAHUD;

	bool bHUDValid = HAHUD && 
	HAHUD->CharacterOverlay &&
	HAHUD->CharacterOverlay->HealthBar && 
	HAHUD->CharacterOverlay->HealthText;
	
	if(bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		HAHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		HAHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void AHAPlayerController::SetHUDKills(float Kills)
{
	HAHUD = HAHUD == nullptr ? Cast<AHAHUD>(GetHUD()) : HAHUD;

	bool bHUDValid = HAHUD &&
		HAHUD->CharacterOverlay &&
		HAHUD->CharacterOverlay->StatsKills;

	if (bHUDValid)
	{
		FString KillsText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Kills));
		HAHUD->CharacterOverlay->StatsKills->SetText(FText::FromString(KillsText));
	}
}

void AHAPlayerController::SetHUDDeaths(int32 Deaths)
{
	HAHUD = HAHUD == nullptr ? Cast<AHAHUD>(GetHUD()) : HAHUD;

	bool bHUDValid = HAHUD &&
		HAHUD->CharacterOverlay &&
		HAHUD->CharacterOverlay->StatsDeaths;

	if (bHUDValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trying to accsess HUD and set deaths: %d"), Deaths);
		FString DeathsText = FString::Printf(TEXT("%d"), Deaths);
		HAHUD->CharacterOverlay->StatsDeaths->SetText(FText::FromString(DeathsText));
	}
}

void AHAPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	HAHUD = HAHUD == nullptr ? Cast<AHAHUD>(GetHUD()) : HAHUD;

	bool bHUDValid = HAHUD &&
		HAHUD->CharacterOverlay &&
		HAHUD->CharacterOverlay->WeaponAmmo;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		HAHUD->CharacterOverlay->WeaponAmmo->SetText(FText::FromString(AmmoText));
	}
}


void AHAPlayerController::SetHUDAmmoOfType(int32 Ammo)
{
	HAHUD = HAHUD == nullptr ? Cast<AHAHUD>(GetHUD()) : HAHUD;

	bool bHUDValid = HAHUD &&
		HAHUD->CharacterOverlay &&
		HAHUD->CharacterOverlay->AmmoOfType;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		HAHUD->CharacterOverlay->AmmoOfType->SetText(FText::FromString(AmmoText));
	}
}

void AHAPlayerController::SetNumericValueInTextBlock(float Value, UTextBlock* TextBlock)
{
	HAHUD = HAHUD == nullptr ? Cast<AHAHUD>(GetHUD()) : HAHUD;

	bool bHUDValid = HAHUD &&
		HAHUD->CharacterOverlay &&
		TextBlock;

	if (bHUDValid)
	{
		FString ValueText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Value));
		TextBlock->SetText(FText::FromString(ValueText));
	}
}
