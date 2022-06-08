// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/HAPlayerController.h"
#include "HUD/HAHUD.h"
#include "HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Character/HABaseCharacter.h"
#include "Components/Image.h"
#include "Net/UnrealNetwork.h"
#include "GameMode//HAGameMode.h"
#include "HUD/Announcment.h"
#include "Kismet/GameplayStatics.h"
#include "HUD/InGameMenu.h"
#include "GameState/HAGameState.h"
#include <HATypes/Announcment.h>


void AHAPlayerController::BeginPlay()
{
	Super::BeginPlay();

	HAHUD = Cast<AHAHUD>(GetHUD());
	ServerCheckMatchState();
}

void AHAPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if(InputComponent == nullptr) return;

	InputComponent->BindAction("InGameMenu", IE_Pressed, this, &AHAPlayerController::ShowInGameMenu);
}

/**
*	Inputs 
*/

void AHAPlayerController::ShowInGameMenu()
{
	if(InGameMenuWidget == nullptr) return;
	if(InGameMenu == nullptr)
	{
		InGameMenu = CreateWidget<UInGameMenu>(this, InGameMenuWidget);
	}
	if(InGameMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if(bReturnToMainMenuOpen)
		{
			InGameMenu->MenuSetup();
		}
		else
		{
			InGameMenu->MenuTearDown();
		}
	}
}

void AHAPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHAPlayerController, MatchState);
	DOREPLIFETIME(AHAPlayerController, bShowTeamScores);
}

void AHAPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	
	CheckTimeSync(DeltaTime);
	PollInit();

	CheckPing(DeltaTime);
}

void AHAPlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
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


void AHAPlayerController::SetHUDScoreTarget(int32 NewTargetScore)
{
	HAHUD = HAHUD == nullptr ? Cast<AHAHUD>(GetHUD()) : HAHUD;

	bool bHUDValid = HAHUD &&
		HAHUD->CharacterOverlay &&
		HAHUD->CharacterOverlay->TargetScoreText;

	if (bHUDValid)
	{
		FString TargetScoreString = FString::Printf(TEXT("%d"), NewTargetScore);
		HAHUD->CharacterOverlay->TargetScoreText->SetText(FText::FromString(TargetScoreString));
	}
}

/**
*  Team Scores
*/

void AHAPlayerController::HideTeamScores()
{
	HAHUD = HAHUD == nullptr ? Cast<AHAHUD>(GetHUD()) : HAHUD;

	bool bHUDValid = HAHUD &&
		HAHUD->CharacterOverlay &&
		HAHUD->CharacterOverlay->YellowTeamScoreText &&
		HAHUD->CharacterOverlay->YellowTeamScoreBar &&
		HAHUD->CharacterOverlay->GreenTeamScoreText &&
		HAHUD->CharacterOverlay->GreenTeamScoreBar;

	if (bHUDValid)
	{
		HAHUD->CharacterOverlay->YellowTeamScoreBar->SetVisibility(ESlateVisibility::Hidden);
		HAHUD->CharacterOverlay->GreenTeamScoreBar->SetVisibility(ESlateVisibility::Hidden);
		HAHUD->CharacterOverlay->YellowTeamScoreText->SetVisibility(ESlateVisibility::Hidden);
		HAHUD->CharacterOverlay->GreenTeamScoreText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AHAPlayerController::InitTeamScores()
{
	HAHUD = HAHUD == nullptr ? Cast<AHAHUD>(GetHUD()) : HAHUD;

	bool bHUDValid = HAHUD &&
		HAHUD->CharacterOverlay &&
		HAHUD->CharacterOverlay->YellowTeamScoreText &&
		HAHUD->CharacterOverlay->YellowTeamScoreBar &&
		HAHUD->CharacterOverlay->GreenTeamScoreText &&
		HAHUD->CharacterOverlay->GreenTeamScoreBar;

	if (bHUDValid)
	{
		HAHUD->CharacterOverlay->YellowTeamScoreBar->SetVisibility(ESlateVisibility::Visible);
		HAHUD->CharacterOverlay->GreenTeamScoreBar->SetVisibility(ESlateVisibility::Visible);
		HAHUD->CharacterOverlay->YellowTeamScoreText->SetVisibility(ESlateVisibility::Visible);
		HAHUD->CharacterOverlay->GreenTeamScoreText->SetVisibility(ESlateVisibility::Visible);

		FString Zero("0");
		HAHUD->CharacterOverlay->YellowTeamScoreBar->SetPercent(0.f);
		HAHUD->CharacterOverlay->GreenTeamScoreBar->SetPercent(0.f);
		HAHUD->CharacterOverlay->YellowTeamScoreText->SetText(FText::FromString(Zero));
		HAHUD->CharacterOverlay->GreenTeamScoreText->SetText(FText::FromString(Zero));
	}
}

void AHAPlayerController::SetHUDYellowTeamScore(int32 YellowScore, int32 TargetScore)
{
	HAHUD = HAHUD == nullptr ? Cast<AHAHUD>(GetHUD()) : HAHUD;

	bool bHUDValid = HAHUD &&
		HAHUD->CharacterOverlay &&
		HAHUD->CharacterOverlay->YellowTeamScoreText &&
		HAHUD->CharacterOverlay->YellowTeamScoreBar;

	if (bHUDValid)
	{
		const float YellowPercent = (float)YellowScore / (float)TargetScore;
		HAHUD->CharacterOverlay->YellowTeamScoreBar->SetPercent(YellowPercent);
		FString YellowScoreString = FString::Printf(TEXT("%d"), YellowScore);
		HAHUD->CharacterOverlay->YellowTeamScoreText->SetText(FText::FromString(YellowScoreString));
	}
}

void AHAPlayerController::SetHUDGreenTeamScore(int32 GreenScore, int32 TargetScore)
{
	HAHUD = HAHUD == nullptr ? Cast<AHAHUD>(GetHUD()) : HAHUD;

	bool bHUDValid = HAHUD &&
		HAHUD->CharacterOverlay &&
		HAHUD->CharacterOverlay->GreenTeamScoreText &&
		HAHUD->CharacterOverlay->GreenTeamScoreBar;

	if (bHUDValid)
	{
		const float GreenPercent = (float)GreenScore / (float)TargetScore;
		HAHUD->CharacterOverlay->GreenTeamScoreBar->SetPercent(GreenPercent);
		FString GreenScoreString = FString::Printf(TEXT("%d"), GreenScore);
		HAHUD->CharacterOverlay->GreenTeamScoreText->SetText(FText::FromString(GreenScoreString));
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDKills = Kills;
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDeaths = Deaths;
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

void AHAPlayerController::SetHUDAnnouncmentTimer(float Time)
{
	HAHUD = HAHUD == nullptr ? Cast<AHAHUD>(GetHUD()) : HAHUD;

	bool bHUDValid = HAHUD &&
		HAHUD->Announcment &&
		HAHUD->Announcment->WarmupTime;

	if (bHUDValid)
	{
		int32 Minuts = FMath::FloorToInt(Time / 60.0f);
		int32 Seconds = Time - Minuts * 60;

		FString TimerText = FString::Printf(TEXT("%02d : %02d"), Minuts, Seconds);
		HAHUD->Announcment->WarmupTime->SetText(FText::FromString(TimerText));
	}
}

void AHAPlayerController::SetHUDAnnouncmentWinner(ETeam WinnerTeam, int32 YellowTeamScore, int32 GreenTeamScore)
{
	HAHUD = HAHUD == nullptr ? Cast<AHAHUD>(GetHUD()) : HAHUD;

	bool bHUDValid = HAHUD &&
		HAHUD->Announcment &&
		HAHUD->Announcment->InfoText &&
		HAHUD->Announcment->GreenScoreText &&
		HAHUD->Announcment->GreenTeamText &&
		HAHUD->Announcment->YellowScoreText &&
		HAHUD->Announcment->YellowTeamText;

	if (bHUDValid)
	{
		FString InfoText("");
		switch (WinnerTeam)
		{
		case ETeam::ET_YellowTeam:
			InfoText = Announcment::YellowTeamWin;
			break;
		case ETeam::ET_GreenTeam:
			InfoText = Announcment::GreenTeamWin;
			break;
		case ETeam::ET_NoTeam:
			InfoText = Announcment::NoWinner;
			break;
		}

		HAHUD->Announcment->InfoText->SetText(FText::FromString(InfoText));
		HAHUD->Announcment->GreenScoreText->SetText(FText::FromString(FString::Printf(TEXT("%d"), GreenTeamScore)));
		HAHUD->Announcment->GreenTeamText->SetText(FText::FromString(Announcment::GreenTeamScore));
		HAHUD->Announcment->YellowScoreText->SetText(FText::FromString(FString::Printf(TEXT("%d"), YellowTeamScore)));
		HAHUD->Announcment->YellowTeamText->SetText(FText::FromString(Announcment::YellowTeamScore));
	}
}

void AHAPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + RoundTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + RoundTime - GetServerTime() + LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if(TimerInt != SecondsLeft)
	{
		if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncmentTimer(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDTimer(TimeLeft);
		}	
	}

	TimerInt = SecondsLeft;
}

void AHAPlayerController::PollInit()
{
	if(CharacterOverlay == nullptr)
	{
		if(HAHUD && HAHUD->CharacterOverlay)
		{
			CharacterOverlay = HAHUD->CharacterOverlay;
			if(CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDKills(HUDKills);
				SetHUDDeaths(HUDDeaths);

				AHAGameState* HAGameState = GetWorld()->GetGameState<AHAGameState>();
				if(HAGameState)
				{
					SetHUDScoreTarget(HAGameState->TargetScore);
				}
			}
		}
	}
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

void AHAPlayerController::ServerCheckMatchState_Implementation()
{
	AHAGameMode* GameMode = Cast<AHAGameMode>(UGameplayStatics::GetGameMode(this));
	if(GameMode)
	{
		LevelStartingTime = GameMode->LevelStartingTime;
		WarmupTime = GameMode->WarmupTime;
		RoundTime = GameMode->RoundTime;
		CooldownTime = GameMode->CooldownTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, RoundTime, CooldownTime, LevelStartingTime);

		if(HAHUD && MatchState == MatchState::WaitingToStart)
		{
			HAHUD->AddAnnouncment();
		}
	}
}

void AHAPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float WarmupT, float RoundT, float CooldownT, float StartingT)
{
	LevelStartingTime = StartingT;
	WarmupTime = WarmupT;
	RoundTime = RoundT;
	CooldownTime = CooldownT;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);

	if (HAHUD && MatchState == MatchState::WaitingToStart)
	{
		if(HasAuthority()) return;
		HAHUD->AddAnnouncment();
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

void AHAPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch /*= false*/)
{
	MatchState = State;

	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if(MatchState == MatchState::Cooldown)
	{
		 HandleCooldown();
	}
}


void AHAPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AHAPlayerController::HandleMatchHasStarted(bool bTeamsMatch /*= false*/)
{
	if(HasAuthority()) bShowTeamScores = bTeamsMatch;
	HAHUD = HAHUD == nullptr ? Cast<AHAHUD>(GetHUD()) : HAHUD;
	if (HAHUD)
	{
		if(HAHUD->CharacterOverlay == nullptr) HAHUD->AddCharacterOverlay();
		if (HAHUD->Announcment)
		{
			HAHUD->Announcment->SetVisibility(ESlateVisibility::Hidden);
		}

		if(!HasAuthority()) return;

		if(bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}

void AHAPlayerController::HandleCooldown()
{
	HAHUD = HAHUD == nullptr ? Cast<AHAHUD>(GetHUD()) : HAHUD;
	if (HAHUD)
	{
		HAHUD->CharacterOverlay->RemoveFromParent();

		bool bHUDValid =
			HAHUD->Announcment &&
			HAHUD->Announcment->AnnouncmentText;
		
		if (bHUDValid)
		{
			HAHUD->Announcment->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncmentText("New Match Starting In: ");
			HAHUD->Announcment->AnnouncmentText->SetText(FText::FromString(AnnouncmentText));
		}

		AHAGameState* HAGameState = Cast<AHAGameState>(UGameplayStatics::GetGameState(this));
		if(HAGameState)
		{
			SetHUDAnnouncmentWinner(HAGameState->WinningTeam, HAGameState->YellowTeamScore, HAGameState->GreenTeamScore);
		}
	}

	AHABaseCharacter* HACharacter = Cast<AHABaseCharacter>(GetPawn());
	if(HACharacter)
	{
		//Mb disable shooting
		HACharacter->bDisableCombat = true;
	}
}


