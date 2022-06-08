// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InGameMenu.generated.h"

class UButton;
class UMultiplayerSessionsSubsystem;
class APlayerController;

UCLASS()
class HEXARENA_API UInGameMenu : public UUserWidget
{
	GENERATED_BODY()
	
public: 
	void MenuSetup();
	void MenuTearDown();

protected:
	virtual bool Initialize() override;

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	UFUNCTION()
	void OnPlayerLeftGame();

private:

	UPROPERTY(meta = (BindWidget))
	UButton* ReturnToMMButton;

	UFUNCTION()
	void ReturnToMMButtonClicked();

	UPROPERTY()
	UMultiplayerSessionsSubsystem* MPSSubsystem;

	UPROPERTY()
	APlayerController* PlayerController;
};
