// Fill out your copyright notice in the Description page of Project Settings.


#include "HAComponents/Inventory.h"

UInventory::UInventory()
{
	PrimaryComponentTick.bCanEverTick = false;

}


void UInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{

}

void UInventory::BeginPlay()
{
	Super::BeginPlay();

	//if()
}


