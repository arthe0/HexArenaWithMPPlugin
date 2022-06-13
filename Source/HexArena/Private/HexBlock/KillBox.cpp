// Fill out your copyright notice in the Description page of Project Settings.


#include "HexBlock/KillBox.h"
#include "Components/BoxComponent.h"
#include <Character/HABaseCharacter.h>
#include "Kismet/GameplayStatics.h"
#include "Pickups/BasePickup.h"
#include <Engine/EngineTypes.h>

AKillBox::AKillBox()
{
	PrimaryActorTick.bCanEverTick = false;

	KillBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	KillBoxComponent->SetupAttachment(GetRootComponent());
}

void AKillBox::BeginPlay()
{
	Super::BeginPlay();
	KillBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AKillBox::OnKillBoxOverlap);
}

void AKillBox::OnKillBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AHABaseCharacter* HACharacter = Cast<AHABaseCharacter>(OtherActor);
	if (HACharacter)
	{
		HACharacter->TakeDamage(1000.f, FDamageEvent(UDamageType::StaticClass()), GetInstigatorController(), this);
		//UGameplayStatics::ApplyDamage(OtherActor, 1000.f, this->GetInstigatorController(), this, UDamageType::StaticClass());
		UE_LOG(LogTemp, Warning, TEXT("KillBox being overlapped with %s"), *OtherActor->GetName());
		return;
	}

	ABasePickup* Pickup = Cast<ABasePickup>(OtherActor);
	if(Pickup)
	{
		Pickup->Destroy(true);
		return;
	}
}




