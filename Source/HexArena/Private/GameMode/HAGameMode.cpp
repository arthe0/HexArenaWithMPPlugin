// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/HAGameMode.h"
#include "Character/HABaseCharacter.h"
#include "PlayerController/HAPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "PlayerStates//HaPlayerState.h"
#include "HexBlock/HexBlock.h"
#include "Pickups/BasePickup.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

AHAGameMode::AHAGameMode()
{
	bDelayedStart = true;
}

void AHAGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();

	//Looking for all HexBlocks at level
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHexBlock::StaticClass(), FoundActors);
	
	//Upcasting AActors to HexBlocks
	TArray<AHexBlock*> MapBlocks;
	for (AActor* Actor: FoundActors)
	{
		if(Actor)
		{
			MapBlocks.Add(Cast<AHexBlock>(Actor));
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Blocks in Map: %d"), MapBlocks.Num());
	//Storing all Blocks by group in TMap
	for (AHexBlock* HexBlock : MapBlocks)
	{
		if(HexBlock)
		{
			if(BlockGroups.Contains(HexBlock->BlockGroup))
			{
				BlockGroups.Find(HexBlock->BlockGroup)->Add((HexBlock));
				UE_LOG(LogTemp, Warning, TEXT("Find Ref, adding to group"));
			}
			else
			{
				TArray<AHexBlock*> BlockGroup;
				BlockGroup.Add(HexBlock);
				BlockGroups.Add(HexBlock->BlockGroup, BlockGroup);
				UE_LOG(LogTemp, Warning, TEXT("Ref not found, creating"));
			}
		}
	}
}

void AHAGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + RoundTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}

		MoveEventTimer+=DeltaTime;
		if(MoveEventTimer>=EventFrequency)
		{
			MoveEvent();
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + RoundTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}

}

void AHAGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AHAPlayerController* Player = Cast<AHAPlayerController>(*It);
		if(Player)
		{
			Player->OnMatchStateSet(MatchState);
		}
	}
}

void AHAGameMode::PlayerEliminated(class AHABaseCharacter* Eliminated, AHAPlayerController* EliminatedPC, AHAPlayerController* AttackerPC)
{
	AHaPlayerState* AttackerPlayerState = AttackerPC ? Cast<AHaPlayerState>(AttackerPC->PlayerState) : nullptr;
	AHaPlayerState* EliminatedPlayerState = EliminatedPC ? Cast<AHaPlayerState>(EliminatedPC->PlayerState) : nullptr;

	if(AttackerPlayerState && AttackerPlayerState != EliminatedPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}

	if(EliminatedPlayerState)
	{
		EliminatedPlayerState->AddToDeaths(1);
	}

	if(Eliminated)
	{
		Eliminated->Death();
	}
}

void AHAGameMode::RequestRespawn(class AHABaseCharacter* Eliminated, AController* EliminatedPC)
{
	if(Eliminated)
	{
		Eliminated->Reset();
		Eliminated->Destroy();
	}
	if(EliminatedPC)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 RandomSelect = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(EliminatedPC, PlayerStarts[RandomSelect]);
	}
}

void AHAGameMode::MoveEvent()
{	
	float RandNum;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABasePickup::StaticClass(), FoundActors);
	for(auto& Actor : FoundActors)
	{
		ABasePickup* FoundPickup = Cast<ABasePickup>(Actor);
		if(FoundPickup)
		{
			if(FoundPickup->GetPhysicsMesh()->IsAnyRigidBodyAwake()) return;
			FoundPickup->GetPhysicsMesh()->WakeAllRigidBodies();
		}
	}

	for (auto& BlockGroup : BlockGroups)
	{
		RandNum = FMath::FRandRange(.0f, 1.f);
		if(RandNum < EventProbability)
		{
			switch (BlockGroup.Value[0]->BlockState)
			{
			case EBlockState::EBS_Default:
				if (FMath::FRandRange(.0f, 1.f) > .5f)
				{
					for (AHexBlock* Block : BlockGroup.Value)
					{
						Block->RiseBlock();
					}
				}
				else
				{
					for (auto& Block : BlockGroup.Value)
					{
						Block->LowerBlock();
					}
				}
			break;

			case EBlockState::EBS_Lowered:
				if (FMath::FRandRange(.0f, 1.f) > .5f)
				{
					for (AHexBlock* Block : BlockGroup.Value)
					{
						Block->RiseBlock();
					}
				}
				else
				{
					for (AHexBlock* Block : BlockGroup.Value)
					{
						Block->StartPosition();
					}
				}
			break;

			case EBlockState::EBS_Rised:
				if (FMath::FRandRange(.0f, 1.f) > .5f)
				{
					for (AHexBlock* Block : BlockGroup.Value)
					{
						Block->LowerBlock();
					}
				}
				else
				{
					for (AHexBlock* Block : BlockGroup.Value)
					{
						Block->StartPosition();
					}
				}
			break;
			}
		}
	}

	MoveEventTimer = 0.f;
}


