// Fill out your copyright notice in the Description page of Project Settings.


#include "HAComponents/LagCompensationComponent.h"
#include "Character/HABaseCharacter.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/BaseWeapon.h"
#include "../HexArena.h"
#include "Weapon/HitBoxTypes.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();


}


void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<AHABaseCharacter>(GetOwner()) : Character;
	if(Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		for(auto& BoxPair : Character->HitBoxes)
		{
			FBoxParams BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			Package.HitBoxParams.Add(BoxPair.Key, BoxInformation);
		}
	}
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	for (auto& YoungerPair : YoungerFrame.HitBoxParams)
	{
		const FName& BoxParamName = YoungerPair.Key;

		const FBoxParams& OlderBox = OlderFrame.HitBoxParams[BoxParamName];
		const FBoxParams& YoungerBox = YoungerFrame.HitBoxParams[BoxParamName];

		FBoxParams InterpBoxParams;

		InterpBoxParams.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
		InterpBoxParams.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
		InterpBoxParams.BoxExtent = YoungerBox.BoxExtent;

		InterpFramePackage.HitBoxParams.Add(BoxParamName, InterpBoxParams);
	}

	return InterpFramePackage;
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, FColor Color)
{
	for (auto& BoxInfo : Package.HitBoxParams)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			4.f
		);
	}
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(AHABaseCharacter* HitCharacter, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensation() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistroy.GetHead() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistroy.GetTail() == nullptr;

	if (bReturn) return FFramePackage();
	// Frame package to verify hit
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;
	// Frame history of the HitCharacter
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensation()->FrameHistroy;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;

	if (OldestHistoryTime > HitTime)
	{
		return FFramePackage();
	}
	if (OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	if (NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;

	while (Older->GetValue().Time > HitTime)
	{
		if (Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();

		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	if (Older->GetValue().Time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}
	if (bShouldInterpolate)
	{
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}

	FrameToCheck.Character = HitCharacter;
	return FrameToCheck;
}


void ULagCompensationComponent::CacheBoxPositions(AHABaseCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if(HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitBoxes)
	{
		if(HitBoxPair.Value !=nullptr)
		{
			FBoxParams BoxParams;
			BoxParams.Location = HitBoxPair.Value->GetComponentLocation();
			BoxParams.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxParams.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxParams.Add(HitBoxPair.Key, BoxParams);

		}
	}
}


void ULagCompensationComponent::MoveBoxes(AHABaseCharacter* HitCharacter, const FFramePackage& Package)
{
	if(HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxParams[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxParams[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxParams[HitBoxPair.Key].BoxExtent);

		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(AHABaseCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxParams[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxParams[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxParams[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(AHABaseCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}

bool ULagCompensationComponent::CheckIfBoxHitted(AHABaseCharacter* HitCharacter, FPredictProjectilePathParams& PathParams, FPredictProjectilePathResult& PathResult, FName BoxName)
{
	return false;
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package, AHABaseCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& Initialvelocity, float HitTime)
{
	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	UHitBoxComponent* HitBox = HitCharacter->HitBoxes[FName("HeadBox")];
	HitBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HitBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithCollision = true;
	PathParams.MaxSimTime = MaxRecordTime;
	PathParams.LaunchVelocity = Initialvelocity;
	PathParams.StartLocation = TraceStart;
	PathParams.SimFrequency = 15.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.TraceChannel = ECC_HitBox;
	PathParams.ActorsToIgnore.Add(GetOwner());
	PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;

	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

	FServerSideRewindResult SSRResult;

	if (PathResult.HitResult.bBlockingHit) // Check hit box
	{
		if (PathResult.HitResult.Component.IsValid())
		{
			UHitBoxComponent* Box = Cast<UHitBoxComponent>(PathResult.HitResult.Component);
			if (Box)
			{
				DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
				SSRResult.bHitConfirmed = true;
				SSRResult.HittedBox = Box->HitBoxType;
			}
		}

		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		return SSRResult;
	}

	else
	{
		for (auto& HitBoxPair : HitCharacter->HitBoxes)
		{
			if(HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}

		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		if(PathResult.HitResult.bBlockingHit)
		{
			if (PathResult.HitResult.Component.IsValid())
			{
				UHitBoxComponent* Box = Cast<UHitBoxComponent>(PathResult.HitResult.Component);
				if (Box)
				{
					DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
					SSRResult.bHitConfirmed = true;
					SSRResult.HittedBox = Box->HitBoxType;
				}
			}
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return SSRResult;
		}
	}

	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	SSRResult.bHitConfirmed = false;
	SSRResult.HittedBox = EHitBoxType::EBHT_NoHit;
	return SSRResult;
}

void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(AHABaseCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& Initialvelocity, float HitTime)
{
	FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, Initialvelocity, HitTime);

	if(Character && HitCharacter && Confirm.bHitConfirmed)
	{
		float DamageToCause = 0.f;
		FWeaponData EquippedWeaponData = Character->GetEquippedWeapon()->WeaponData;
		switch (Confirm.HittedBox)
		{
		case EHitBoxType::EHBT_Head:
			DamageToCause = EquippedWeaponData.BaseDamage * EquippedWeaponData.HeadMultiplyer;
			break;

		case EHitBoxType::EHBT_Neck:
			DamageToCause = EquippedWeaponData.BaseDamage * EquippedWeaponData.NeckMultiplyer;
			break;
	
		case EHitBoxType::EHBT_Chest:
			DamageToCause = EquippedWeaponData.BaseDamage * EquippedWeaponData.ChestMultiplyer;
			break;

		case EHitBoxType::EHBT_Stomach:
			DamageToCause = EquippedWeaponData.BaseDamage * EquippedWeaponData.StomachMultiplyer;
			break;

		case EHitBoxType::EHBT_Limbs:
			DamageToCause = EquippedWeaponData.BaseDamage * EquippedWeaponData.LimbsMultiplyer;
			break;
		}

		UE_LOG(LogTemp, Warning, TEXT("Applying damage with SSR on PSSR: %s to %s"), *this->GetName(), *HitCharacter->GetName());

		UGameplayStatics::ApplyDamage(
			HitCharacter,
			DamageToCause,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(AHABaseCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

// Called every frame
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(FrameHistroy.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistroy.AddHead(ThisFrame);
	}
	else
	{
		float HistoryLenght = FrameHistroy.GetHead()->GetValue().Time - FrameHistroy.GetTail()->GetValue().Time;

		while (HistoryLenght>MaxRecordTime)
		{
			FrameHistroy.RemoveNode(FrameHistroy.GetTail());
			HistoryLenght = FrameHistroy.GetHead()->GetValue().Time - FrameHistroy.GetTail()->GetValue().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistroy.AddHead(ThisFrame);

		//ShowFramePackage(ThisFrame, FColor::Green);
	}
}



