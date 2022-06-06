#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupide UMETA(DisplayName = "Unoccupide"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_LoweringWeapon UMETA(DisplayName = "LoweringWeapon"),
	ECS_EquipingWeapon UMETA(DisplayName = "EquipingWeapon"),

	ECS_MAX UMETA(DisplayName = "DefaulMAX")
};
