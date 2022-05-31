// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HAHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	float CrosshairSpread;
	FLinearColor CrosshairColor;
};

class UCharacterOverlay;
class UUserWidget;
class UAnnouncment;

UCLASS()
class HEXARENA_API AHAHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "PlayerStats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	void AddCharacterOverlay();

	UPROPERTY(EditAnywhere, Category = "Announcments")
	TSubclassOf<UUserWidget> AnnouncmentClass;

	UPROPERTY()
	UAnnouncment* Announcment;

	void AddAnnouncment();

private:
	virtual void BeginPlay()override;

	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 15.f;
public:
	FORCEINLINE void SetHUDPackage (const FHUDPackage& Package) {HUDPackage = Package; }
};
