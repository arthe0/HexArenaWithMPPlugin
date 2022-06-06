// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/AmmoPickup.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Pickups/BasePickup.h"
#include "../HexArena.h"

AAmmoPickup::AAmmoPickup()
{
	bReplicates = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMeshComponent"));
	StaticMeshComponent->SetupAttachment(GetRootComponent());

	StaticMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	StaticMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	StaticMeshComponent->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	StaticMeshComponent->MarkRenderStateDirty();
	EnableCustomDepth(true);

	bAutoPickup = true;

	PickupType = EPickupTypes::EPT_Ammo;
}

void AAmmoPickup::BeginPlay()
{
	Super::BeginPlay();

	StaticMeshComponent->SetStaticMesh(AmmoPickupData.Mesh);
}

FName AAmmoPickup::GetAmmoName_Implementation()
{
	return AmmoName;
}

/**
 * Cosmetics
 */

void AAmmoPickup::EnableCustomDepth(bool bEnable)
{
	if (StaticMeshComponent)
	{
		StaticMeshComponent->SetRenderCustomDepth(bEnable);
	}
}
