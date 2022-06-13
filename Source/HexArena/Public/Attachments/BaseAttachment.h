// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "BaseAttachment.generated.h"

#define CUSTOM_DEPTH_PURPLE 250

class UStaticMesh;

UENUM(BlueprintType)
enum class EAttachmentType : uint8
{
	EAT_Sight UMETA(DisplayName = "Sight"),
	EAT_Muzzle UMETA(DisplayName = "Muzzle"),
	EAT_Grip UMETA(DisplayName = "Grip"),
	EAT_Mag UMETA(DisplayName = "Mag"),
	EAT_Stock UMETA(DisplayName = "Stock"),

	EAT_MAX UMETA(DisplayName = "DefaultMAX")
};


USTRUCT(BlueprintType)
struct FAttachmentData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "AttachmentType")
	EAttachmentType AttachmentType;

	UPROPERTY(EditAnywhere, Category = "AttachmentType")
	TSubclassOf<ABaseAttachment> AttachmentClass;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition = "A"), Category = "AttachmentMesh")
	UStaticMesh* AttachmentMesh;

	UPROPERTY(EditAnywhere, Category = "Aim")
	bool AffectAiming = false;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "AffectAiming"), Category = "Aim")
	float ZoomInterpSpeedMultiplyer = 1.f;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "AttachmentType == EAttachmentType::EAT_Sight"), Category = "Aim")
	float ZoomedFOVMultiplyer = 1.f;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "AttachmentType == EAttachmentType::EAT_Mag"), Category = "Aim")
	int32 MagCapacity = 30;

	UPROPERTY(EditAnywhere, Category = "Recoil")
	bool AffectRecoil = false;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "AffectRecoil"), Category = "Recoil")
	float RecoilX = 10.f;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "AffectRecoil"), Category = "Recoil")
	float RecoilY = 10.f;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "AffectRecoil"), Category = "Recoil")
	float SpreadMultiplyer = 1.f;
};

UCLASS()
class HEXARENA_API ABaseAttachment : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseAttachment();

	UPROPERTY(EditAnywhere)
	FAttachmentData AttachmentData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* AttachmentMesh;

	UPROPERTY(EditAnywhere)
	FName AttachmentName;

	void EnableCustomDepth(bool bEnable);
	

protected:
	virtual void BeginPlay() override;
	//virtual void SetDataByName();
private:
	//UDataTable* AttachmentsTable;

	
public:
	//void SetAttachmentDataByName(FName NewName);


};
