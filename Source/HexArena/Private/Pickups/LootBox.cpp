// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/LootBox.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "../HexArena.h"
#include "Weapon/BaseWeapon.h"
#include "Pickups/AmmoPickup.h"

ALootBox::ALootBox()
{
	PrimaryActorTick.bCanEverTick = true;

	LootBoxComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LootBoxMesh"));
	SetRootComponent(LootBoxComponent);

	LootBoxComponent->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
	LootBoxComponent->MarkRenderStateDirty();
	EnableCustomDepth(true);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(LootBoxComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Overlap);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(LootBoxComponent);

	PickupWidget->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UDataTable> LootDTObject(TEXT("DataTable'/Game/Blueprints/Pickups/DT_Loot.DT_Loot'"));
	if (LootDTObject.Succeeded())
	{
		LootTable = LootDTObject.Object;
	}
}

void ALootBox::BeginPlay()
{
	Super::BeginPlay();

	GenerateLoot();

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ALootBox::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ALootBox::OnSphereEndOverlap);

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

void ALootBox::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsRefilling)
	{
		RefilTimer += DeltaSeconds;
		if(RefilTimer >= RefilTime)
		{
			bIsRefilling = false;
			GenerateLoot();
		}
	}
}

void ALootBox::OpenBox()
{
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnableCustomDepth(false);
	bIsRefilling = true;
	RefilTimer = 0.f;
	PickupWidget->SetVisibility(false);
	CreateLootItems();
	ServerOpenBox();
}

void ALootBox::ServerOpenBox_Implementation()
{
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnableCustomDepth(false);
}

void ALootBox::GenerateLoot()
{
	if(!HasAuthority()) return
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	LootBoxComponent->MarkRenderStateDirty();
	EnableCustomDepth(true);

	if(LootTable)
	{
		TArray<FName> RowNames = LootTable->GetRowNames();
		for (int32 Pickups = 0; Pickups < MaxLootCount; Pickups++)
		{
			int32 RandomNameIndex = FMath::RandRange(0, RowNames.Num()-1);
			FLootData* TempLootRow = LootTable->FindRow<FLootData>(RowNames[RandomNameIndex], "");

			if (TempLootRow)
			{
				Loot.Add(TempLootRow);
			}
		}
	}

	ServerGenerateLoot();
}

void ALootBox::ServerGenerateLoot_Implementation()
{
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	LootBoxComponent->MarkRenderStateDirty();
	EnableCustomDepth(true);
}

void ALootBox::CreateLootItems()
{
	
	UWorld* World = GetWorld();
	if(!World) return; 

	for(int32 I = 0; I<Loot.Num(); I++)
	{
		if(Loot[I]->PickupType == EPickupTypes::EPT_Weapon)
		{
			ABaseWeapon* BaseWeapon = World->SpawnActorDeferred<ABaseWeapon>(Loot[I]->LootClass, this->GetTransform());
			BaseWeapon->SetWeaponDataByName(Loot[I]->LootName);
			BaseWeapon->FinishSpawning(this->GetTransform());
			BaseWeapon->AddImpulse(FVector((50.f - I * 50.f), 100, 150), NAME_None, true);
		}
		else if(Loot[I]->PickupType == EPickupTypes::EPT_Ammo)
		{
			AAmmoPickup* AmmoPickup = World->SpawnActorDeferred<AAmmoPickup>(Loot[I]->LootClass, this->GetTransform());
			AmmoPickup->SetAmmoDataByName(Loot[I]->LootName);
			AmmoPickup->FinishSpawning(this->GetTransform());
			AmmoPickup->AddImpulse(FVector((50.f - I * 50.f), 100, 150), NAME_None, true);
		}
	}
	Loot.Empty();
}
