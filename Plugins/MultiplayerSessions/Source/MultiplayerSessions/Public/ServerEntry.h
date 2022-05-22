// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "../Plugins/Online/OnlineSubsystem/Source/Public/OnlineSessionSettings.h"
#include "ServerEntry.generated.h"

class UTextBlock;
class UButton;
class Umenu;
class FOnlineSessionSearchResult;

UCLASS()
class MULTIPLAYERSESSIONS_API UServerEntry : public UUserWidget
{
	GENERATED_BODY()

public:
	//UFUNCTION(BlueprintCallable)
	void ServerDataSetup(UMenu* Widget, FOnlineSessionSearchResult Session, FText ServerRegion, FText ServerName, int32 CurrentPlayers, int32 MaxPlayers, int32 ServerPing, FText ServerStatus);

	
private:
	virtual bool Initialize() override;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ServerRegionText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ServerNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ServerPlayersText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ServerPingText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ServerStatusText;

	UPROPERTY(meta = (BindWidget))
	UButton* SelectServerButton;

	FOnlineSessionSearchResult ThisSession;

	UPROPERTY()
	UMenu* OwnerWidget;

	UFUNCTION()
	void OnSelectServerButtonClicked();
	/*FText ServerRegion;
	FText ServerName;
	int32 CurrentPlayers;
	int32 MaxPlayers;
	int32 ServerPing;
	FText ServerStatus;*/

};
