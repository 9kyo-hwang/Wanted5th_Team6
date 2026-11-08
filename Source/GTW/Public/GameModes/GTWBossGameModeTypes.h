#pragma once

#include "CoreMinimal.h"
#include "GTWBossGameModeTypes.generated.h"

UENUM(BlueprintType)
enum class EBossGameModeState : uint8
{
	Ready	UMETA(DisplayName = "READY"),
	Play	UMETA(DisplayName = "PLAY"),
	Clear	UMETA(DisplayName = "CLEAR"),
	Fail	UMETA(DisplayName = "FAIL")
};

UENUM(BlueprintType)
enum class EBossPhase1OutroTypes : uint8
{
	Fall,
	Cleared
};

UENUM(BlueprintType)
enum class EBossPhase2OutroTypes : uint8
{
	Fall,
	BossContact,
	Cleared
};
