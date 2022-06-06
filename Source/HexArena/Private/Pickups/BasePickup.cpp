// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/BasePickup.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/HABaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../HexArena.h"

ABasePickup::ABasePickup()
{
	PrimaryActorTick.bCanEverTick = false;

	PhysicsMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PhysicsMeshComponent"));
	SetRootComponent(PhysicsMeshComponent);

	PhysicsMeshComponent->SetSimulatePhysics(true);
	PhysicsMeshComponent->SetEnableGravity(true);
	PhysicsMeshComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(PhysicsMeshComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Overlap);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(AreaSphere);

}

void ABasePickup::AddImpulse(FVector Vector, FName BoneName, bool bVelocity)
{
	PhysicsMeshComponent->AddImpulse(Vector, BoneName, bVelocity);
}

void ABasePickup::BeginPlay()
{
	Super::BeginPlay();

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ABasePickup::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ABasePickup::OnSphereEndOverlap);

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}	
}



