// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerEntry.h"
#include "Components/TextBlock.h"
#include "Menu.h"
#include "Components/Button.h"



void UServerEntry::ServerDataSetup(UMenu* Widget, FOnlineSessionSearchResult Session, FText ServerRegion, FText ServerName, int32 CurrentPlayers, int32 MaxPlayers, int32 ServerPing, FText ServerStatus)
{
	if(Widget)
	{	
		OwnerWidget = Widget;
	}
	
	ThisSession = Session;
	
	
	if(ServerRegionText)
	{
		ServerRegionText->SetText(ServerRegion);
	}

	if(ServerNameText)
	{
		ServerNameText->SetText(ServerName);
	}

	if(ServerPlayersText)
	{
		FString Players = FString::Printf(TEXT("%d/%d"), CurrentPlayers, MaxPlayers);
		ServerPlayersText->SetText(FText::FromString(Players));
	}

	if(ServerPlayersText)
	{	
		FString Ping = FString::Printf(TEXT("%d"), ServerPing);
		ServerPingText->SetText(FText::FromString(Ping));
	}

	if(ServerStatusText)
	{
		ServerStatusText->SetText(ServerStatus);
	}
}

bool UServerEntry::Initialize()
{
	if (!Super::Initialize()) return false;
	if(SelectServerButton)
	{
		SelectServerButton->OnClicked.AddDynamic(this, &ThisClass::OnSelectServerButtonClicked);
	}
	return true;
}

void UServerEntry::OnSelectServerButtonClicked()
{
	OwnerWidget->SetSessionToJoin(ThisSession);
}
