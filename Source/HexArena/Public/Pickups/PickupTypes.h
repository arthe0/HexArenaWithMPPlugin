#pragma once

UENUM(BlueprintType)
enum class EPickupTypes : uint8
{
	EPT_Weapon UMETA(DisplayName = "Weapon"),
	EPT_Ammo UMETA(DisplayName = "Ammo"),
	EPT_PowerUp UMETA(DisplayName = "PowerUP"),

	EPT_MAX UMETA(DisplayName = "DefaultMax"),
};

