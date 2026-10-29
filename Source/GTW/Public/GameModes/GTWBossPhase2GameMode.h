// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include <GameModes/GTWBossGameModeTypes.h>
#include "GTWBossPhase2GameMode.generated.h"

/**
 * 보스 2페이즈 레벨에서 사용하는 게임 모드
 * - 성공 판정: 맵 끝에 도달 시(목표 Trigger Overlap)
 */
UCLASS()
class GTW_API AGTWBossPhase2GameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void StartPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Phase")
	void InitGameplay(class ACharacter* InBoss);

	UFUNCTION(BlueprintCallable, Category = "Boss|Phase")
	void StartGameplay();

	// 플레이어가 목표 지점에 도달했을 때(Trigger Overlap) 호출할 함수
	UFUNCTION(BlueprintCallable, Category="Boss|Phase")
	void OnGoalReached();

	/*
	* 낙하 판정 TriggerVolume OR 보스 접촉 시 호출될 함수
	*/
	UFUNCTION(BlueprintCallable, Category = "Boss|Phase")
	void FailPhase();

protected:
	/*
	TODO 리스트
	1. 보스 패턴 시작
	2. 맵 패턴 액터 가동
	3. Player 조작 시작
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Phase", meta = (DisplayName = "On Gameplay Started"))
	void K2_OnGameplayStarted();

	/*
	TODO 리스트
	1. 보스 공격, 예약 실행 중지
	2. 환경 기믹 액터들 정리(타이머, 판정 OFF 등)
	3. 게임플레이 이벤트(콜백) 정리
	4. 플레이어 조작 Block
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Phase", meta = (DisplayName = "On Gameplay Stopped"))
	void K2_OnGameplayStopped();

	// 결과 확정 + 정리 완료 후 연출 등 후속 처리 요청
	// Clear: 플레이타임 기록 + 클리어 화면 재생(?)
	// Fail:  Phase 1 레벨 다시 Open
	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Phase", meta = (DisplayName = "On Phase Finished"))
	void K2_OnPhaseFinished(EBossGameModeState Result);

private:
	void ClearPhase();
	void StopGameplay();

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Boss|Phase", meta = (AllowPrivateAccess = "true"))
	EBossGameModeState CurrentState = EBossGameModeState::Ready;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ACharacter> Boss;
};
