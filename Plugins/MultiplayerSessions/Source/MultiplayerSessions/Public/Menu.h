// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Components/SizeBox.h"
#include "Menu.generated.h"

class UButton;
class USizeBox;
class UScrollBox;
class FOnlineSessionSearchResult;

UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int NumberOfPublicConnections = 4, FString TypeOFMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/LobbyLevel")));

	UPROPERTY(EditDefaultsOnly)
	int32 NumSerachResults = 10000;
protected:
	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

	//
	//Callbacks for custom delegates on the MultiplayerSessionSubsystem
	//
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

	FOnlineSessionSearchResult* SessionToJoin;
private:
	
	/*
	 *	Setup Variables
	 */

	//The sybsystem designed to handle all online session functionality
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	int32 NumPublicConnections{ 4 };
	FString MatchType{ TEXT("FreeForAll") };
	FString PathToLobby{ TEXT("") };

	
	TArray<FOnlineSessionSearchResult> FoundSessionResults;

	/*
	 *	Main Menu Objects
	 */

	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* BrowseButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;

	/*
	 *	Main Menu Functions
	 */

	UFUNCTION()
	void OnHostButtonClicked();

	UFUNCTION()
	void OnBrowseButtonClicked();

	UFUNCTION()
	void SetMenuButtonsVisibility(ESlateVisibility ButtonsVisibility);

	void MenuTearDown();

	/*
	 *	Browser Objects
	 */

	UPROPERTY(meta = (BindWidget))
	USizeBox* ServerBrowserSizeBox;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ServersScrollBox;

	UPROPERTY(meta = (BindWidget))
	UButton* BrowserJoinButton;

	UPROPERTY(meta = (BindWidget))
	UButton* BrowserRefreshButton;

	UPROPERTY(meta = (BindWidget))
	UButton* BrowserCloseButton;

	UPROPERTY(EditAnywhere, Category = "ServerEntryLine")
	TSubclassOf<UUserWidget> ServerEntryLine;

	/*
	 *	Browser Functions
	 */

	UFUNCTION()
	void OnJoinButtonClicked();

	UFUNCTION()
	void OnRefreshButtonClicked();

	UFUNCTION()
	void OnCloseButtonClicked();

public:
	FORCEINLINE void SetSessionToJoin(FOnlineSessionSearchResult& Session) { SessionToJoin = &Session; }
};
