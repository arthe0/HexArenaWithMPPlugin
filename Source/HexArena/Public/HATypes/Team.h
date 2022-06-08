#pragma once

UENUM(BlueprintType)
enum class ETeam : uint8
{
	ET_YellowTeam UMETA(DisplayName = "YellowTeam"),
	ET_GreenTeam UMETA(DisplayName = "GreenTeam"),
	ET_NoTeam UMETA(DisplayName = "NoTeam"),

	ET_Max UMETA(DisplayName = "DefaultMAX")
};