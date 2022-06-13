// Fill out your copyright notice in the Description page of Project Settings.


#include "Attachments/ScopeAttachment.h"
#include "Character/HABaseCharacter.h"
#include "Components/SceneCaptureComponent2D.h"

AScopeAttachment::AScopeAttachment()
{
	ScopeCamera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("ScopeCamera"));
	ScopeCamera->SetupAttachment(AttachmentMesh);	
}

void AScopeAttachment::BeginPlay()
{
	Super::BeginPlay();
	ScopeCamera->FOVAngle = ScopeFOV;
}

void AScopeAttachment::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
	if(NewOwner == nullptr)
	{
		ResetRenderTarget();
	}
	else
	{
		AHABaseCharacter* HACharacater = Cast<AHABaseCharacter>(GetOwner());
		if (!HACharacater || !HACharacater->IsLocallyControlled()) return;
		ClinetSetRenderPlayer();
	}
}

void AScopeAttachment::SetScopeFOV(float NewFov)
{
	ScopeFOV = NewFov;
	ScopeCamera->FOVAngle = ScopeFOV;
}

void AScopeAttachment::ClinetSetRenderPlayer_Implementation()
{
	
	UE_LOG(LogTemp, Warning, TEXT("SetRender to %s, With role: %d"), *GetOwner()->GetName(), GetOwner()->GetLocalRole());
	if (RenderTarget)
	{
		ScopeCamera->TextureTarget = RenderTarget;
	}
	
}


void AScopeAttachment::ResetRenderTarget()
{
	if (!RenderTarget) return;
	ScopeCamera->TextureTarget = nullptr;
}

