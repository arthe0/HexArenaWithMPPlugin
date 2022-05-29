// Fill out your copyright notice in the Description page of Project Settings.


#include "HexBlock/HexBlock.h"
#include "Components/StaticMeshComponent.h"

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
	

	//if (LowerCurve)
	//{
	//	LowerTimeline.AddInterpFloat(LowerCurve, TimelimeFloat);
	//}
}

void AHexBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RiseTimeline.TickTimeline(DeltaTime);
	//LowerTimeline.TickTimeline(DeltaTime);
}

void AHexBlock::TimelineProgress(float Value)
{
	FVector NewLocation = FMath::Lerp(CurrentLocation, MoveToLocation, Value);
	SetActorLocation(NewLocation);
	UE_LOG(LogTemp, Warning, TEXT("Rising!"));
}

void AHexBlock::RiseBlock()
{
	CurrentLocation = GetActorLocation();
	MoveToLocation = RiseLocation;
	RiseTimeline.PlayFromStart();
	UE_LOG(LogTemp, Warning, TEXT("Rising block!"));
}

void AHexBlock::LowerBlock()
{
	CurrentLocation = GetActorLocation();
	MoveToLocation = LowerLocation;
	RiseTimeline.PlayFromStart();
}

void AHexBlock::StartPosition()
{
	CurrentLocation = GetActorLocation();
	MoveToLocation = DefaultLocation;
	RiseTimeline.PlayFromStart();
}



