// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/Interactable.h"
#include "Character/HABaseCharacter.h"
#include "Components/WidgetComponent.h"

AInteractable::AInteractable()
{
	PrimaryActorTick.bCanEverTick = false;

}

void AInteractable::BeginPlay()
{
	Super::BeginPlay();
	
}

void AInteractable::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AHABaseCharacter* HACharacter = Cast<AHABaseCharacter>(OtherActor);
	if (HACharacter)
	{
		HACharacter->SetOverlappingPickup(this);
		UE_LOG(LogTemp, Warning, TEXT("Item %s, was overlapped"), *this->GetName())
	}
}

void AInteractable::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AHABaseCharacter* HACharacter = Cast<AHABaseCharacter>(OtherActor);
	if (HACharacter)
	{
		HACharacter->SetOverlappingPickup(nullptr);
	}
}

void AInteractable::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

