// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <GameModes/GTWBaseBossGameMode.h>
#include "GTWBossPhase2GameMode.generated.h"

/**
 * 보스 2페이즈 레벨에서 사용하는 게임 모드
 * - 성공 판정: 맵 끝에 도달 시(목표 Trigger Overlap)
 */

UENUM(BlueprintType)
enum class EBossPhase2FailReason : uint8
{
	Fall,
	BossContact,
};

UCLASS()
class GTW_API AGTWBossPhase2GameMode : public AGTWBaseBossGameMode
{
	GENERATED_BODY()

public:
	AGTWBossPhase2GameMode();

	// Clear()에다가 다이렉트로 Overlap 연결

	// 2페이즈는 실패 종류가 2가지라 Enum을 받음
	UFUNCTION(BlueprintCallable, Category="Boss|Phase")
	void Fail(EBossPhase2FailReason InFailReason);

protected:
	virtual UClass* GetBossClass() const override;
	virtual void RecordCombatResult(EBossGameModeState Result) override;

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Boss|Phase", meta=(AllowPrivateAccess = "true"))
	EBossPhase2FailReason FailReason;
};
