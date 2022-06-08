// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class AHABaseCharacter;
class UDamageType;
class AController;
class AHAGameMode;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HEXARENA_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();

	friend AHABaseCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


protected:
	virtual void BeginPlay() override;
	void UpdateHUDHealth();

	void Heal(float HealValue);

private:

	AHABaseCharacter* Character;
	/*
	* Player Health
	*/

	UPROPERTY(EditAnywhere, Category = "Player Sats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Sats")
	float Health = 100.f;

	UPROPERTY(EditAnywhere, Category = "Regeneration")
	float TimeToRegen = 5.f;

	UPROPERTY(EditAnywhere, Category = "Regeneration")
	float Frequency = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Regeneration")
	float HealAmount = 5.0f;

	bool bNeedAutoHealing = false;
	float LastHitTime = 0.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	UFUNCTION()
	void OnTakeAnyDamageHandle(AActor* DamageActor, float Damage,
	const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY()
	AHAGameMode* HAGameMode;

public:	
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float NewHealth) { Health = NewHealth; }

};
