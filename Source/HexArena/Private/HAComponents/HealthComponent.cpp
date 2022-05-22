

#include "HAComponents/HealthComponent.h"
#include "Character/HABaseCharacter.h"
#include "PlayerController/HAPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "GameMode//HAGameMode.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	if(Character)
	{
		UpdateHUDHealth();

		if (Character->HasAuthority())
		{
			Character->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::OnTakeAnyDamageHandle);
		}
	}
}

void UHealthComponent::UpdateHUDHealth()
{
	Character->HAPlayerController = Character->GetPlayerController();
	if (Character->HAPlayerController && Health)
	{
		Character->HAPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, Health);
}

void UHealthComponent::OnRep_Health()
{
	UpdateHUDHealth();
	Character->PlayHitReactMontage();
}

void UHealthComponent::OnTakeAnyDamageHandle(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	
	if(Character)
	{
		Character->PlayHitReactMontage();
		UpdateHUDHealth();
	}
	if(Health <=0.f)
	{
		Character->bDeath = true;
		AHAGameMode* HAGameMode = GetWorld()->GetAuthGameMode<AHAGameMode>();
		if (HAGameMode)
		{
			AHAPlayerController* InstigatorController = Cast<AHAPlayerController>(InstigatedBy);
			HAGameMode->PlayerEliminated(Character, Character->GetPlayerController(), InstigatorController);
		}
	}
}



void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

