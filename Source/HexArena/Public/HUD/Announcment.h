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
};
