#pragma once

#include "CoreMinimal.h"
#include "Actor/Gimmicks/GTWJumpTileActor.h"
#include "GTWP1TornadoJumpPad.generated.h"

/** 2층 점프/효과음은 그대로 사용하고 P1 INI의 점프 강도만 전달한다. */
UCLASS(HideCategories = (Interaction))
class GTW_API AGTWP1TornadoJumpPad : public AGTWJumpTileActor
{
	GENERATED_BODY()

protected:
	virtual void PostInitializeComponents() override;
};
