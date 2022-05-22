#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	EWT_SMG UMETA(DisplayName = "SMG"),
	EWT_Rifle UMETA(DisplayName = "Rifle"),
	EWT_Sniper UMETA(DisplayName = "Sniper"),
	EWT_Launcher UMETA(DisplayName = "Launcher"),

	EWT_MAX UMETA(DisplayName = "DefaulMAX")
};
