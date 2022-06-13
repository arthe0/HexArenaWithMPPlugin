// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Attachments/BaseAttachment.h"
#include "ScopeAttachment.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTArget2D;

UCLASS()
class HEXARENA_API AScopeAttachment : public ABaseAttachment
{
	GENERATED_BODY()
public:
	AScopeAttachment();

	UPROPERTY(EditAnywhere, Category = "Aim")
	float ScopeFOV = 30.f;

	UFUNCTION(Client, Reliable)
	void ClinetSetRenderPlayer();
	void ResetRenderTarget();

protected:
	virtual void BeginPlay() override;
	virtual void SetOwner( AActor* NewOwner ) override;

	UPROPERTY(EditAnywhere, Category = "Aim")
	USceneCaptureComponent2D* ScopeCamera;

	UPROPERTY(EditAnywhere, Category = "Aim")
	UTextureRenderTarget2D* RenderTarget;

private:

public:
	void SetScopeFOV(float NewFov);

	
};
