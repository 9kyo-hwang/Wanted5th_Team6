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
