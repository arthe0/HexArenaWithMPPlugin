// Fill out your copyright notice in the Description page of Project Settings.


#include "HexBlock/HexBlock.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

AHexBlock::AHexBlock()
{
	PrimaryActorTick.bCanEverTick = true;
	HexMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HexStaticMesh"));
	HexMeshComponent->SetupAttachment(GetRootComponent());
	SetRootComponent(HexMeshComponent);
}

void AHexBlock::BeginPlay()
{
	Super::BeginPlay();

	if(RiseCurve)
	{
		FOnTimelineFloat TimelimeFloat;
		TimelimeFloat.BindUFunction(this, FName("TimelineProgress"));
		RiseTimeline.AddInterpFloat(RiseCurve, TimelimeFloat);
	}

	DefaultLocation = GetActorLocation();
	CurrentLocation = GetActorLocation();
	RiseLocation = GetActorLocation();
	LowerLocation = GetActorLocation();

	RiseLocation.Z += MovingMultiplyer;
	LowerLocation.Z -= MovingMultiplyer;
}


void AHexBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RiseTimeline.TickTimeline(DeltaTime);
}

void AHexBlock::TimelineProgress(float Value)
{
	FVector NewLocation = FMath::Lerp(CurrentLocation, MoveToLocation, Value);
	ServerTimelineProgress(NewLocation);
}

void AHexBlock::ServerTimelineProgress_Implementation(const FVector_NetQuantize& NewLocation)
{
	MulticastTimelineProgress(NewLocation);
}

void AHexBlock::MulticastTimelineProgress_Implementation(const FVector_NetQuantize& NewLocation)
{
	SetActorLocation(NewLocation);
}

void AHexBlock::RiseBlock()
{
	BlockState = EBlockState::EBS_Rised;
	CurrentLocation = GetActorLocation();
	MoveToLocation = RiseLocation;
	RiseTimeline.PlayFromStart();
}

void AHexBlock::LowerBlock()
{
	BlockState = EBlockState::EBS_Lowered;
	CurrentLocation = GetActorLocation();
	MoveToLocation = LowerLocation;
	RiseTimeline.PlayFromStart();
}

void AHexBlock::StartPosition()
{
	BlockState = EBlockState::EBS_Default;
	CurrentLocation = GetActorLocation();
	MoveToLocation = DefaultLocation;
	RiseTimeline.PlayFromStart();
}



