// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcment.generated.h"


class UTextBlock;

UCLASS()
class HEXARENA_API UAnnouncment : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WarmupTime;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnnouncmentText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* InfoText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GreenScoreText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GreenTeamText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* YellowScoreText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* YellowTeamText;
};
