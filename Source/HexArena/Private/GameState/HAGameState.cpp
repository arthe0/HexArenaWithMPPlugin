// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/HAGameState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/HAPlayerController.h"
#include "GameMode/HAGameMode.h"

void AHAGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHAGameState, GreenTeamScore);
	DOREPLIFETIME(AHAGameState, YellowTeamScore);
	DOREPLIFETIME(AHAGameState, TargetScore);
}

void AHAGameState::OnRep_TargetScore()
{
	AHAPlayerController* HAPController = Cast<AHAPlayerController>(GetWorld()->GetFirstPlayerController());
	if (HAPController)
	{
		HAPController->SetHUDScoreTarget(TargetScore);
	}
}

void AHAGameState::YellowTeamScores()
{
	YellowTeamScore++;
	AHAPlayerController* HAPController = Cast<AHAPlayerController>(GetWorld()->GetFirstPlayerController());
	if(HAPController)
	{
		HAPController->SetHUDYellowTeamScore(YellowTeamScore, TargetScore);
	}
	if(YellowTeamScore>GreenTeamScore)
	{
		WinningTeam = ETeam::ET_YellowTeam;
	}
	else if(YellowTeamScore == GreenTeamScore)
	{
		WinningTeam = ETeam::ET_NoTeam;
	}
	if(YellowTeamScore >= TargetScore)
	{ 
		OnTeamWin.ExecuteIfBound(ETeam::ET_YellowTeam);
	}
}

void AHAGameState::GreenTeamScores()
{
	GreenTeamScore++;
	AHAPlayerController* HAPController = Cast<AHAPlayerController>(GetWorld()->GetFirstPlayerController());
	if (HAPController)
	{
		HAPController->SetHUDGreenTeamScore(GreenTeamScore, TargetScore);
	}
	if (YellowTeamScore < GreenTeamScore)
	{
		WinningTeam = ETeam::ET_GreenTeam;
	}
	else if (YellowTeamScore == GreenTeamScore)
	{
		WinningTeam = ETeam::ET_NoTeam;
	}
	if (GreenTeamScore >= TargetScore)
	{
		OnTeamWin.ExecuteIfBound(ETeam::ET_GreenTeam);
	}
}

void AHAGameState::OnRep_YellowTeamScore()
{
	AHAPlayerController* HAPController = Cast<AHAPlayerController>(GetWorld()->GetFirstPlayerController());
	if (HAPController)
	{
		HAPController->SetHUDYellowTeamScore(YellowTeamScore, TargetScore);
	}
}

void AHAGameState::OnRep_GreenTeamScore()
{
	AHAPlayerController* HAPController = Cast<AHAPlayerController>(GetWorld()->GetFirstPlayerController());
	if (HAPController)
	{
		HAPController->SetHUDGreenTeamScore(GreenTeamScore, TargetScore);
	}
}

void AHAGameState::SetTargetScore(int32 NewTargetScore)
{
	TargetScore = NewTargetScore;
	AHAPlayerController* HAPController = Cast<AHAPlayerController>(GetWorld()->GetFirstPlayerController());
	if (HAPController)
	{
		HAPController->SetHUDScoreTarget(TargetScore);
	}
}

