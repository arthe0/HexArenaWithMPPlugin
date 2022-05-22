// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/HAAnimInstance.h"
#include "Character//HABaseCharacter.h"
#include "GameFramework//CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Weapon/BaseWeapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "HATypes/CombatState.h"


void UHAAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	HACharacter = Cast<AHABaseCharacter>(TryGetPawnOwner());
	if(HACharacter)
	{
		CharacterMesh = HACharacter->GetMesh();
	}
}

void UHAAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if(!HACharacter)
	{
		HACharacter = Cast<AHABaseCharacter>(TryGetPawnOwner());
	}
	if(HACharacter == nullptr) return; 


	FVector Velocity = HACharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = HACharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = HACharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = HACharacter->IsWeaponEquipped();
	EquippedWeapon = HACharacter->GetEquippedWeapon();
	bIsCrouched = HACharacter->bIsCrouched;
	bAiming = HACharacter->IsAiming();
	bDeath = HACharacter->bIsDeath();
	MovementDirection = HACharacter->GetMovementDirection();
	TurningInPlace = HACharacter->GetTurningInPlace();

	//AO_Yaw = HACharacter->GetAO_Yaw();
	AO_Pitch = HACharacter->GetAO_Pitch();

	if(bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && HACharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		HACharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if(HACharacter->IsLocallyControlled())
		{ 
			bLocllyControlled = true;
			FTransform RightHandTrnsform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTrnsform.GetLocation(), RightHandTrnsform.GetLocation() + (RightHandTrnsform.GetLocation() - HACharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation,LookAtRotation, DeltaTime, 15.f);
		}
		/* Debugging aim and muzzle directions
		FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX*1000.f, FColor::Red);
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), HACharacter->GetHitTarget(), FColor::Orange);*/
	}

	bReloading = HACharacter->GetCombatState() == ECombatState::ECS_Reloading;

	//FPS Properties START
	//TODO: Promote this functional to delegate in future
	if(EquippedWeapon)
	{
		IKProperties = EquippedWeapon->IKProperties;
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UHAAnimInstance::SetIKTransforms);
	}
	SetVars(DeltaTime);
	//FPS Properties END
}

void UHAAnimInstance::SetVars(const float DelataTime)
{
	CameraTransform = FTransform(HACharacter->GetBaseAimRotation(), HACharacter->GetCameraComponent()->GetComponentLocation());

	const FTransform& RootOffset = CharacterMesh->GetSocketTransform(FName("root"), RTS_Component).Inverse() * CharacterMesh->GetSocketTransform(FName("ik_hand_root"));
	RelativeCameraTransform = CameraTransform.GetRelativeTransform(RootOffset);
	ADSWeight = HACharacter->GetADSWeight();
}

void UHAAnimInstance::CalculateWeaponSway(const float DeltaTime)
{

}

void UHAAnimInstance::SetIKTransforms()
{
	RightHandToSightTransform = EquippedWeapon->GetsightsWorldTransform().GetRelativeTransform(CharacterMesh->GetSocketTransform(FName("hand_r")));
}
