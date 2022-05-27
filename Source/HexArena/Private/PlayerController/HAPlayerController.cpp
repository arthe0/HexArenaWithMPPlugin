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
	SetHUDTime();
	
	CheckTimeSync(DeltaTime);

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
			//UE_LOG(LogTemp, Warning, TEXT("PlayerState->GetCompressedPing() * 4 = %d"), PlayerState->GetCompressedPing() * 4);
			if (PlayerState->GetCompressedPing() * 4 > HighPingThreshold) // Ping is compressed by 4 by default
			{
				HighPingWarning();
				PlayAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
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


void AHAPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
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

void AHAPlayerController::SetHUDTimer(float Time)
{
	HAHUD = HAHUD == nullptr ? Cast<AHAHUD>(GetHUD()) : HAHUD;

	bool bHUDValid = HAHUD &&
		HAHUD->CharacterOverlay &&
		HAHUD->CharacterOverlay->TimerText;

	if (bHUDValid)
	{
		int32 Minuts = FMath::FloorToInt(Time / 60.0f);
		int32 Seconds = Time - Minuts * 60;

		FString TimerText = FString::Printf(TEXT("%02d : %02d"), Minuts, Seconds);
		HAHUD->CharacterOverlay->TimerText->SetText(FText::FromString(TimerText));
	}
}

void AHAPlayerController::SetHUDTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(RoundTime - GetServerTime());
	if(TimerInt != SecondsLeft)
	{
		SetHUDTimer(RoundTime - GetServerTime());
	}

	TimerInt = SecondsLeft;
}

float AHAPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	return GetWorld()->GetTimeSeconds() + ClientServerDeltaTime;
}

void AHAPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AHAPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReciept = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReciept);
}

void AHAPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerRecivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerRecivedClientRequest + SingleTripTime;
	ClientServerDeltaTime = CurrentServerTime - GetWorld()->GetTimeSeconds();
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


void AHAPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if(IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}