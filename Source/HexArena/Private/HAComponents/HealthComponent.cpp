

#include "HAComponents/HealthComponent.h"
#include "Character/HABaseCharacter.h"
#include "PlayerController/HAPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "GameMode//HAGameMode.h"
#include <HexBlock/KillBox.h>

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Character)
	{
		UpdateHUDHealth();

		if (Character->HasAuthority())
		{
			Character->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::OnTakeAnyDamageHandle);
		}
	}
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, Health);
}

void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bNeedAutoHealing || Character == nullptr || Character->bIsDeath()) return;
	if (LastHitTime + TimeToRegen <= GetWorld()->GetTimeSeconds())
	{
		if (Health < MaxHealth)
		{
			Heal(HealAmount * DeltaTime / Frequency);
		}
		else
		{
			bNeedAutoHealing = false;
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

void UHealthComponent::Heal(float HealValue)
{
	SetHealth(FMath::Clamp(Health+HealAmount, 0.f, MaxHealth));
	UpdateHUDHealth();
}

void UHealthComponent::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if(LastHealth>Health)
	{
		Character->PlayHitReactMontage();
	}
}

void UHealthComponent::OnTakeAnyDamageHandle(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	HAGameMode = HAGameMode == nullptr ? GetWorld()->GetAuthGameMode<AHAGameMode>() : HAGameMode;
	if(Character->bDeath || HAGameMode == nullptr) return;
	if(Character->Controller == nullptr) return;
	if(!Cast<AKillBox>(DamageCauser))
	{
		Damage = HAGameMode->CalculateDamage(InstigatedBy, Character->Controller, Damage);
	}
	
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	LastHitTime = GetWorld()->GetTimeSeconds();
	bNeedAutoHealing = true;

	if(Character)
	{
		Character->PlayHitReactMontage();
	}
	if(Health <=0.f)
	{
		Character->bDeath = true;
		HAGameMode = HAGameMode == nullptr ? GetWorld()->GetAuthGameMode<AHAGameMode>() : HAGameMode;
		if (HAGameMode)
		{
			AHAPlayerController* InstigatorController = Cast<AHAPlayerController>(InstigatedBy);
			HAGameMode->PlayerEliminated(Character, Character->GetPlayerController(), InstigatorController);
		}
		
	}
	UpdateHUDHealth();
}

