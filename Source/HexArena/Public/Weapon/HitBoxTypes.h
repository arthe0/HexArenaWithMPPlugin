#pragma once

UENUM(BlueprintType)
enum class EHitBoxType : uint8
{
	EHBT_Head UMETA(DisplayName = "HeadHB"),
	EHBT_Neck UMETA(DisplayName = "NeckHB"),
	EHBT_Chest UMETA(DisplayName = "ChestHB"),
	EHBT_Stomach UMETA(DisplayName = "StomachHB"),
	EHBT_Limbs UMETA(DisplayName = "LimbsHB"),
	EBHT_NoHit UMETA(DisplayName = "NoHit"),

	EHBT_MAX UMETA(DisplayName = "DefaulMAX")
};