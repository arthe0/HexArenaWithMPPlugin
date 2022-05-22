// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Components/ScrollBox.h"
#include "OnlineSubsystem.h"
#include "ServerEntry.h"

void UMenu::MenuSetup(int NumberOfPublicConnections, FString TypeOFMatch, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOFMatch;

	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if(World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if(PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

bool UMenu::Initialize()
{
	if(!Super::Initialize()) return false;

	if(HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::OnHostButtonClicked);		
	}

	if(BrowserJoinButton)
	{
		BrowserJoinButton->OnClicked.AddDynamic(this, &ThisClass::OnJoinButtonClicked);
	}

	if(ServerBrowserSizeBox)
	{
		ServerBrowserSizeBox->SetVisibility(ESlateVisibility::Collapsed);
	}

	if(BrowseButton)
	{
		BrowseButton->OnClicked.AddDynamic(this, &ThisClass::OnBrowseButtonClicked);
	}

	if(BrowserCloseButton)
	{
		BrowserCloseButton->OnClicked.AddDynamic(this, &ThisClass::OnCloseButtonClicked);
	}

	return true;
}

void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

/*
*	Custom Callbacks
*/

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Green,
				FString(TEXT("Session Created Successfuly!"))
			);
		}

		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToLobby);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Failed to create session!"))
			);
		}
		HostButton->SetIsEnabled(true);
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if(MultiplayerSessionsSubsystem == nullptr)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("MPSessionSubsystem not found!"))
			);
		}
		return;
	}
	FoundSessionResults = SessionResults;
	int32 ServerIndex = 0;
	for(auto Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);


		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString::Printf(TEXT("Find session: %s"), *Result.Session.OwningUserName)
			);
		}

		if(SettingsValue == MatchType)
		{	
			UServerEntry* NewServer = CreateWidget<UServerEntry>(this, ServerEntryLine);
			int32 ServerPublicConnsections;  
			Result.Session.SessionSettings.Get(FName("NumPublicConnections"), ServerPublicConnsections);
			NewServer->ServerDataSetup(
				this,
				Result,
				FText::FromString("RU"),
				FText::FromString(Result.Session.OwningUserName),
				1,
				ServerPublicConnsections,
				Result.PingInMs,
				FText::FromString("In lobby")
			);

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Red,
					FString::Printf(TEXT("Find match session: %s"), *Result.Session.OwningUserName)
				);
			}

			if(ServersScrollBox && NewServer)
			{
				NewServer->SetVisibility(ESlateVisibility::Visible);
				ServersScrollBox->AddChild(NewServer);
			}
			ServerIndex++;
			return;
		}
	}
	if(!bWasSuccessful || SessionResults.Num() == 0)
	{
		BrowserJoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();

	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if(SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();

			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
	if(Result != EOnJoinSessionCompleteResult::Success)
	{
		BrowserJoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{

}

void UMenu::OnStartSession(bool bWasSuccessful)
{

}

/*
*	Main Menu Functions
*/

void UMenu::OnHostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
		
	}
}

void UMenu::OnBrowseButtonClicked()
{
	SetMenuButtonsVisibility(ESlateVisibility::Collapsed);
	if(ServerBrowserSizeBox)
	{
		ServerBrowserSizeBox->SetVisibility(ESlateVisibility::Visible);

		if(MultiplayerSessionsSubsystem)
		{
			MultiplayerSessionsSubsystem->FindSessions(NumSerachResults);
		}
		
	}
}

void UMenu::SetMenuButtonsVisibility(ESlateVisibility ButtonsVisibility)
{
	if(HostButton)
	{
		HostButton->SetVisibility(ButtonsVisibility);
	}
	
	if(BrowseButton)
	{
		BrowseButton->SetVisibility(ButtonsVisibility);
	}
	
	if(ExitButton)
	{
		ExitButton->SetVisibility(ButtonsVisibility);
	}
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputData;
			PlayerController->SetInputMode(InputData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

/*
*	Browser Functions
*/


void UMenu::OnJoinButtonClicked()
{
	BrowserJoinButton->SetIsEnabled(false);
	
	if(!SessionToJoin) return;
	MultiplayerSessionsSubsystem->JoinSession(*SessionToJoin);
}

void UMenu::OnRefreshButtonClicked()
{
	if (ServersScrollBox)
	{
		ServersScrollBox->ClearChildren();;
	}
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(NumSerachResults);
	}
}

void UMenu::OnCloseButtonClicked()
{	
	SetMenuButtonsVisibility(ESlateVisibility::Visible);
	if(ServersScrollBox)
	{
		ServersScrollBox->ClearChildren();;
	}
	if(ServerBrowserSizeBox)
	{
		ServerBrowserSizeBox->SetVisibility(ESlateVisibility::Collapsed);
	}
	SessionToJoin = nullptr;
}


