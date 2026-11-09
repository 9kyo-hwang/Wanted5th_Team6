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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhase2OutroRequestedDelegate, EBossPhase2OutroTypes, OutroType);

UCLASS()
class GTW_API AGTWBossPhase2GameMode : public AGTWBaseBossGameMode
{
	GENERATED_BODY()

public:
	AGTWBossPhase2GameMode();

	// 플레이어가 목표 지점에 도달했을 때(Trigger Overlap) 호출할 함수
	UFUNCTION(BlueprintCallable, Category="Boss|Phase")
	void OnGoalReached();

	// 2페이즈는 실패 종류가 2가지라 Enum을 받음
	UFUNCTION(BlueprintCallable, Category="Boss|Phase")
	void FailPhase(EBossPhase2FailReason InFailReason);

	// 보스 아웃트로 요청
	UPROPERTY(BlueprintAssignable, Category = "Boss|Phase")
	FOnPhase2OutroRequestedDelegate OnOutroRequested;

protected:
	virtual bool BindBossEvents() override;
	virtual void UnbindBossEvents() override;
	virtual void HandlePhaseResult(EBossGameModeState Result) override;
	virtual bool RequestPhaseOutro(EBossGameModeState Result) override;
	virtual bool HandlePhaseCompletion(EBossGameModeState Result) override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Level", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UWorld> Phase1Level;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Level", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UWorld> ClearLevel;

	EBossPhase2FailReason FailReason;
};
