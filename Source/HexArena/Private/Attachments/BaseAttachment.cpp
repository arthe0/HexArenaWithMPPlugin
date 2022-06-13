// Fill out your copyright notice in the Description page of Project Settings.


#include "Attachments/BaseAttachment.h"

ABaseAttachment::ABaseAttachment()
{
	PrimaryActorTick.bCanEverTick = false;

	AttachmentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachmentMesh"));
	SetRootComponent(AttachmentMesh);

	/*static ConstructorHelpers::FObjectFinder<UDataTable> LootDTObject(TEXT("DataTable'/Game/Blueprints/Weapon/Attachment/DT_Attachments.DT_Attachments'"));
	if (LootDTObject.Succeeded())
	{
		AttachmentsTable = LootDTObject.Object;
	}*/

	AttachmentMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	AttachmentMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
}

void ABaseAttachment::BeginPlay()
{
	Super::BeginPlay();
	//SetDataByName();
}

//void ABaseAttachment::SetDataByName()
//{
//	if (AttachmentsTable)
//	{
//		AttachmentData = *AttachmentsTable->FindRow<FAttachmentData>(AttachmentName, "");
//	}
//
//	if(AttachmentData.AttachmentMesh)
//	{
//		AttachmentMesh->SetStaticMesh(AttachmentData.AttachmentMesh);
//	}
//}
//
//void ABaseAttachment::SetAttachmentDataByName(FName NewName)
//{
//	AttachmentName = NewName;
//	SetDataByName();
//}


void ABaseAttachment::EnableCustomDepth(bool bEnable)
{
	if (AttachmentMesh)
	{
		AttachmentMesh->SetRenderCustomDepth(bEnable);
	}
}

