#pragma once

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_Light UMETA(DisplayName = "Light Ammo"),
	EAT_Shotgun UMETA(DisplayName = "Shotgun Ammo"),
	EAT_Rifle UMETA(DisplayName = "Rifle Ammo"),
	EAT_Sniper UMETA(DisplayName = "Sniper Ammo"),
	EAT_Launcher UMETA(DisplayName = "Launcher Ammo"),

	EAT_MAX UMETA(DisplayName = "DefaulMAX")
};
