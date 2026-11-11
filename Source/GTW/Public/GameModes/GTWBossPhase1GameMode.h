// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <GameModes/GTWBaseBossGameMode.h>
#include "GTWBossPhase1GameMode.generated.h"


/**
 * 보스 1페이즈 레벨에서 사용되는 게임 모드
 * - TODO: 4층 소코반/슈팅 게임 클리어 결과에 따라 보스의 난이도가 바뀜
 * - 성공: P1 보스가 계산한 QR 제출 목표 달성 / 실패: 맵 밖으로 떨어져서 바닥에 깔린 TriggerBox에 플레이어가 닿으면
 */

UCLASS()
class GTW_API AGTWBossPhase1GameMode : public AGTWBaseBossGameMode
{
	GENERATED_BODY()

public:
	AGTWBossPhase1GameMode();

	UFUNCTION(BlueprintCallable, Category = "Boss|State")
	void Fail();

protected:
	virtual UClass* GetBossClass() const override;

	virtual bool BindBossEvents() override;
	virtual void UnbindBossEvents() override;
	
	virtual void RecordCombatResult(EBossGameModeState Result) override;

private:
	void OnHitBoss();	// 보스가 유효타 맞았을 때 호출될 콜백. Boss측 OnObjectHit 델리게이트가 발행될 때.

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Boss|Phase", meta = (AllowPrivateAccess = "true"))
	int32 CurrentHitCount = 0;

	FDelegateHandle BossHitDelegateHandle;
};
