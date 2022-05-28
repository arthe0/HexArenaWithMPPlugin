// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	UWorld* World = GetWorld();
	if(MuzzleFlashSocket&& World)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// From muzzle flash socket to hit location from TraceUnderCrosshair
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;

		AProjectile* SpawnedProjectile = nullptr;

		if(bUseSSR)
		{
			if (InstigatorPawn->HasAuthority()) // On Server
			{
				if(InstigatorPawn->IsLocallyControlled()) // Server, host: Using Replicated projectile
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(WeaponData.ProjectileClass,SocketTransform.GetLocation(),TargetRotation,SpawnParams);
					SpawnedProjectile->bUseSSR = false;
					SpawnedProjectile->SetInstigatorWeapon(this);
				}
				else // Server, not Locally controlled: Doesn't Uses Replicated projectile
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseSSR = true;
				}
			}
			else // Client: Using SSR
			{
				if(InstigatorPawn->IsLocallyControlled()) // Client, Locally controller: Using SSR non-rep
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseSSR = true;
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
					SpawnedProjectile->SetInstigatorWeapon(this);
				}
				else // Client, Not Locally Controlled: Non-rep, no SSR
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseSSR = false;
				}
			}
		}
		else // Not using SSR
		{
			if(InstigatorPawn->HasAuthority())
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(WeaponData.ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnedProjectile->bUseSSR = false;
				SpawnedProjectile->SetInstigatorWeapon(this);
			}
		}
	}
}
