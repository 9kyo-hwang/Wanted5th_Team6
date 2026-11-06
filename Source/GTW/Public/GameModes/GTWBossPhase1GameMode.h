// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <GameModes/GTWBaseBossGameMode.h>
#include "GTWBossPhase1GameMode.generated.h"


/**
 * 보스 1페이즈 레벨에서 사용되는 게임 모드
 * - TODO: 4층 소코반/슈팅 게임 클리어 결과에 따라 보스의 난이도가 바뀜
 * - 성공: 보스 유효타 3회 / 실패: 맵 밖으로 떨어져서 바닥에 깔린 TriggerBox에 플레이어가 닿으면
 */

/*
* 1페 보스쪽에 필요한 것
* OnObjectHit: C++ 멀티캐스트(인자 없음)
*/


UCLASS()
class GTW_API AGTWBossPhase1GameMode : public AGTWBaseBossGameMode
{
	GENERATED_BODY()

public:
	AGTWBossPhase1GameMode();
	
protected:
	virtual bool BindBossEvents() override;
	virtual void UnbindBossEvents() override;
	virtual void HandlePhaseResult(EBossGameModeState Result) override;
	virtual bool HandlePhaseCompletion(EBossGameModeState Result) override;

private:
	void OnHitBoss();	// 보스가 유효타 맞았을 때 호출될 콜백. Boss측 OnObjectHit 델리게이트가 발행될 때.

private:
	static constexpr int32 ClearHitCount = 3;	// 3번 맞추면 클리어

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Boss|Phase", meta = (AllowPrivateAccess = "true"))
	int32 CurrentHitCount = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Level", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UWorld> Phase1Level;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Level", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UWorld> Phase2Level;

	FDelegateHandle BossHitDelegateHandle;
};
