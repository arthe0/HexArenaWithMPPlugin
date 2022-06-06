// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HAPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingToHigh);

class AHAHUD;
class UTextBlock;
class UCharacterOverlay;

UCLASS()
class HEXARENA_API AHAPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetHUDHealth (float Health, float MaxHealth);
	void SetHUDKills (float Kills);
	void SetHUDDeaths (int32 Deaths);
	void SetHUDWeaponAmmo (int32 Ammo);
	void SetHUDAmmoOfType (int32 Ammo);
	void SetHUDTimer(float Time);
	void SetHUDAnnouncmentTimer(float Time);
	virtual void OnPossess(APawn* InPawn) override;

	virtual float GetServerTime();
	virtual void ReceivedPlayer() override;
	void OnMatchStateSet(FName State);
	void HandleCooldown();
	void HandleMatchHasStarted();

	float SingleTripTime = 0.f;
	FHighPingDelegate HighPingDelegate;
protected:
	virtual void BeginPlay() override;

	void SetHUDTime();
	void PollInit();

	/**
	* Synchronizing client and server time 
	*/
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float WarmupT, float RoundT, float CooldownT, float StartingT);

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimerServerRecivedClientRequest);

	float ClientServerDeltaTime = 0.f;

	void HighPingWarning();
	void StopHighPingWarning();

	void CheckPing(float DeltaTime);


private:
	AHAHUD* HAHUD;

	void SetNumericValueInTextBlock(float Value, UTextBlock* TextBlock);

	float LevelStartingTime = 0.f;
	float RoundTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	uint32 TimerInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UCharacterOverlay* CharacterOverlay;

	float HighPingRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 10.f;

	float PlayAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 15.f;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus (bool bHighPing);

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 100.f;

	bool bInitializeCharacterOverlay = false;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDKills;
	int32 HUDDeaths;
};
