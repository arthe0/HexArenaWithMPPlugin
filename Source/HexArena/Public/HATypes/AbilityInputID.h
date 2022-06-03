#pragma once

UENUM(BlueprintType)
enum class EHAAbilityInputID : uint8
{
	EHAAIID_None UMETA(DisplayName = "None"),
	EHAAIID_Confirm UMETA(DisplayName = "Confirm"),
	EHAAIID_Cancel UMETA(DisplayName = "Cancel"),
	EHAAIID_Attack UMETA(DisplayName = "Attack"),
	EHAAIID_Jump UMETA(DisplayName = "Jump"),

	ETIP_MAX UMETA(DisplayName = "DefaultMax"),
};