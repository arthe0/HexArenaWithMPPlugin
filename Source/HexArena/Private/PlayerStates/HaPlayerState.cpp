// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerStates/HaPlayerState.h"
#include "Character/HABaseCharacter.h"
#include "PlayerController/HAPlayerController.h"
#include "Net/UnrealNetwork.h"


void AHaPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHaPlayerState, Defeats);
	DOREPLIFETIME(AHaPlayerState, Kills);
	DOREPLIFETIME(AHaPlayerState, Team);
}

void AHaPlayerState::AddToScore(float ScoreAmount)
{
	Kills += ScoreAmount;
	SetScore(Kills);
	Character = Character == nullptr ? Cast<AHABaseCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AHAPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDKills(GetScore());
		}
	}
}

void AHaPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	Character = Character == nullptr ? Cast<AHABaseCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AHAPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDKills(GetScore());
		}
	}
}

void AHaPlayerState::AddToDeaths(int32 DeathsAmount)
{
	Defeats += DeathsAmount;
	Character = Character == nullptr ? Cast<AHABaseCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AHAPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDeaths(Defeats);
			UE_LOG(LogTemp, Warning, TEXT("Add to deaths Server"));
		}
	}
}

void AHaPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<AHABaseCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AHAPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDeaths(Defeats);
			UE_LOG(LogTemp, Warning, TEXT("Add to deaths Client"));
		}
	}
}

void AHaPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;

	Character = Character == nullptr ? Cast<AHABaseCharacter>(GetPawn()) : Character;
	if (Character)
	{
		switch (Team)
		{
		case ETeam::ET_GreenTeam:
			Character->SetTeamName("GreenTeam");
			break;
		case ETeam::ET_YellowTeam:
			Character->SetTeamName("YellowTeam");
			break;
		case ETeam::ET_NoTeam:
			Character->SetTeamName("NoTeam");
			break;
		}
	}
}

void AHaPlayerState::OnRep_Team()
{
	Character = Character == nullptr ? Cast<AHABaseCharacter>(GetPawn()) : Character;
	if (Character)
	{
		switch (Team)
		{
		case ETeam::ET_GreenTeam:
			Character->SetTeamName("GreenTeam");
			break;
		case ETeam::ET_YellowTeam:
			Character->SetTeamName("YellowTeam");
			break;
		case ETeam::ET_NoTeam:
			Character->SetTeamName("NoTeam");
			break;
		}
	}
}



//void AHaPlayerState::OnRep_Team()
//{
//	Team = TeamToSet;
//
//	ABlasterCharacter* BCharacter = Cast <ABlasterCharacter>(GetPawn());
//	if (BCharacter)
//	{
//		BCharacter->SetTeamColor(Team);
//	}
//}
//
//void AHaPlayerState::SetTeam(ETeam TeamToSet)
//{
//	ABlasterCharacter* BCharacter = Cast <ABlasterCharacter>(GetPawn());
//	if (BCharacter)
//	{
//		BCharacter->SetTeamColor(Team);
//	}
//}
