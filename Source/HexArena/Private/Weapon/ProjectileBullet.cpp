// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Character/HABaseCharacter.h"
#include "PlayerController/HAPlayerController.h"
#include "HAComponents/LagCompensationComponent.h"
#include "Weapon/Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Weapon/BaseWeapon.h"

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(struct FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName ProppertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if(ProppertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))
	{
		if(ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("Hit.GetComponent of %s is %s"), *this->GetName(), *Hit.GetComponent()->GetName());
	AHABaseCharacter* OwnerCharacter = Cast<AHABaseCharacter>(GetOwner());
	if(OwnerCharacter)
	{
		AHAPlayerController* OwnerController = Cast<AHAPlayerController>(OwnerCharacter->Controller);
		if(OwnerController)
		{
			if(OwnerCharacter->HasAuthority() && !bUseSSR)
			{
				if(Hit.GetComponent()->IsA(UHitBoxComponent::StaticClass()) && InstigatorWeapon)
				{

					UHitBoxComponent* HitBox = Cast<UHitBoxComponent>(Hit.GetComponent());
					switch (HitBox->HitBoxType)
					{
					case EHitBoxType::EHBT_Head:
						Damage = InstigatorWeapon->WeaponData.BaseDamage * InstigatorWeapon->WeaponData.HeadMultiplyer;
						break;

					case EHitBoxType::EHBT_Neck:
						Damage = InstigatorWeapon->WeaponData.BaseDamage * InstigatorWeapon->WeaponData.NeckMultiplyer;
						break;

					case EHitBoxType::EHBT_Chest:
						Damage = InstigatorWeapon->WeaponData.BaseDamage * InstigatorWeapon->WeaponData.ChestMultiplyer;
						break;

					case EHitBoxType::EHBT_Stomach:
						Damage = InstigatorWeapon->WeaponData.BaseDamage * InstigatorWeapon->WeaponData.StomachMultiplyer;
						break;

					case EHitBoxType::EHBT_Limbs:
						Damage = InstigatorWeapon->WeaponData.BaseDamage * InstigatorWeapon->WeaponData.LimbsMultiplyer;
						break;
					}
				}

				
				UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}
			
			AHABaseCharacter* HitCharacter = Cast<AHABaseCharacter>(OtherActor);
			if(bUseSSR && OwnerCharacter->GetLagCompensation() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
			{
				OwnerCharacter->GetLagCompensation()->ProjectileServerScoreRequest(
					HitCharacter,
					TraceStart,
					InitialVelocity,
					OwnerController->GetServerTime() - OwnerController->SingleTripTime
				);
			}
		}
	}
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
	
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();
}
