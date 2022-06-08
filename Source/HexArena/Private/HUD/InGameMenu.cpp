// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/InGameMenu.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameModeBase.h"
#include <Character/HABaseCharacter.h>

void UInGameMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if(World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if(PlayerController)
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	if (ReturnToMMButton && !ReturnToMMButton->OnClicked.IsBound())
	{
		ReturnToMMButton->OnClicked.AddDynamic(this, &UInGameMenu::ReturnToMMButtonClicked);
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MPSSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MPSSubsystem)
		{
			MPSSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UInGameMenu::OnDestroySession);
		}
	}
}

bool UInGameMenu::Initialize()
{
	if(!Super::Initialize())
	{
		return false;
	}
	return true;
}

void UInGameMenu::MenuTearDown()
{
	this->RemoveFromParent();

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameOnly InputMode;
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(false);
		}
	}

	if (ReturnToMMButton && ReturnToMMButton->OnClicked.IsBound())
	{
		ReturnToMMButton->OnClicked.RemoveDynamic(this, &UInGameMenu::ReturnToMMButtonClicked);
	}
	if(MPSSubsystem && MPSSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
	{
		MPSSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UInGameMenu::OnDestroySession);
	}
}


void UInGameMenu::OnDestroySession(bool bWasSuccessful)
{
	if(!bWasSuccessful)
	{
		ReturnToMMButton->SetIsEnabled(true);
		return;
	}

	UWorld* World = GetWorld();
	if(World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if(GameMode)
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if(PlayerController)
			{
				PlayerController->ClientReturnToMainMenu(FString());
			}
		}
	}
}

void UInGameMenu::ReturnToMMButtonClicked()
{
	ReturnToMMButton->SetIsEnabled(false);
	
	UWorld* World = GetWorld();
	if(World)
	{
		APlayerController* FirstPlayerController = World->GetFirstPlayerController();
		if(FirstPlayerController)
		{
			AHABaseCharacter* HACharacter = Cast<AHABaseCharacter>(FirstPlayerController->GetPawn());
			if (HACharacter)
			{
				HACharacter->ServerLeaveGame();
				HACharacter->OnLeftGame.AddDynamic(this, &UInGameMenu::OnPlayerLeftGame);
			}
			else
			{
				ReturnToMMButton->SetIsEnabled(true);
			}
		}
	}
}

void UInGameMenu::OnPlayerLeftGame()
{
	if (MPSSubsystem)
	{
		MPSSubsystem->DestroySession();
	}
}
