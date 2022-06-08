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

	static ConstructorHelpers::FObjectFinder<UDataTable> LootDTObject(TEXT("DataTable'/Game/Blueprints/Pickups/DT_AmmoPickups.DT_AmmoPickups'"));
	if (LootDTObject.Succeeded())
	{
		AmmoTable = LootDTObject.Object;
	}
}

void AAmmoPickup::BeginPlay()
{
	Super::BeginPlay();
	SetAmmoDataByName(AmmoName);
}

void AAmmoPickup::SetAmmoDataByName(FName NewName)
{
	if (HasAuthority())
	{
		MulticastSetAmmoDataByName(NewName);
	}
}

void AAmmoPickup::MulticastSetAmmoDataByName_Implementation(FName NewName)
{
	AmmoName = NewName;
	if (AmmoTable)
	{
		AmmoPickupData = *AmmoTable->FindRow<FAmmoPickupData>(AmmoName, "");
	}

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
