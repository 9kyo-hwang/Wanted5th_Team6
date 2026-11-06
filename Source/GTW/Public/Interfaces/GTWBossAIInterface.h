// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GTWBossAIInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UGTWBossAIInterface : public UInterface
{
	GENERATED_BODY()
};

// 정확한 이름은 BossCombatInterface
class GTW_API IGTWBossAIInterface
{
	GENERATED_BODY()

public:
	// 보스 공격, 추격 등 시작. GM이 호출할 곳
	virtual void StartCombat() = 0;

	// 추격이나 기믹 활성화 등 Stop.
	virtual void StopCombat() = 0;
};
