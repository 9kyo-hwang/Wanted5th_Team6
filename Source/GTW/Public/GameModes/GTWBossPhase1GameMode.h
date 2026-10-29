// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include <GameModes/GTWBossGameModeTypes.h>
#include "GTWBossPhase1GameMode.generated.h"


/**
 * 보스 1페이즈 레벨에서 사용되는 게임 모드
 * - 준비, 시작, 성공, 실패 상태 존재
 * - 4층 소코반/슈팅 게임 클리어 결과에 따라 보스의 난이도가 바뀜
 * - 성공 판정: 보스 유효타 3회
 */

UCLASS()
class GTW_API AGTWBossPhase1GameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void StartPlay() override;	

public:
	// Level Blueprint에서 GameMode에게 보스, 스포너 참조를 넘겨줄 통로
	// @TODO: Spawner 타입으로 변경해야 함
	UFUNCTION(BlueprintCallable, Category="Boss|Phase")
	void InitGameplay(class ACharacter* InBoss, const TArray<class AActor*>& InHitObjectSpawners);

	// 추후 연출이 들어온다면 연출 완료 시 Callback으로 해당 함수 호출(지금은 Level BeginPlay 시 바로 수행)
	UFUNCTION(BlueprintCallable, Category = "Boss|Phase")
	void StartGameplay();

	// 보스가 유효타 맞았을 때 날리는 통지에서 호출하는 콜백
	UFUNCTION(BlueprintCallable, Category = "Boss|Phase")
	void OnHitBoss();

	/*
	* Map에 깔려있는 TriggerVolume에서 overlap 판정 감지해서 호출하게 될 함수
	* Cast<ACharacter>(OtherActor) -> Cast<BossPhase1GameMode>(GetGameMode()) -> FailPhase()
	*/
	UFUNCTION(BlueprintCallable, Category = "Boss|Phase")
	void FailPhase();

protected:
	/*
	TODO 리스트
	1. 보스 패턴 시작
	2. 스포너 StartSpawning()
	3. 맵 패턴 액터 가동
	4. Player 조작 시작
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Phase", meta=(DisplayName="On Gameplay Started"))
	void K2_OnGameplayStarted();
	
	/*
	TODO 리스트
	1. 보스 공격, 예약 실행 중지
	2. Spawner->StopSpawning()
	3. 환경 기믹 액터들 정리(타이머, 판정 OFF 등)
	4. 게임플레이 이벤트(콜백) 정리
	5. 플레이어 조작 Block
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Phase", meta = (DisplayName = "On Gameplay Stopped"))
	void K2_OnGameplayStopped();

	// 결과 확정 + 정리 완료 후 연출 등 후속 처리 요청
	// Clear: 전환 연출 실행 -> 완료 시 Callback으로 Phase 2 레벨 Open
	// Fail:  Phase 1 레벨 다시 Open
	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Phase", meta = (DisplayName = "On Phase Finished"))
	void K2_OnPhaseFinished(EBossGameModeState Result);

private:
	void ClearPhase();	// 보스 유효타 기반 판정 처리
	void StopGameplay();	// 성공 또는 실패 시 맵에 있는 액터들을 중지(정리)하는 역할

private:
	static constexpr int32 ClearHitCount = 3;	// 3번 맞추면 클리어
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Boss|Phase", meta=(AllowPrivateAccess = "true"))
	EBossGameModeState CurrentState = EBossGameModeState::Ready;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Boss|Phase", meta = (AllowPrivateAccess = "true"))
	int32 CurrentHitCount = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Boss", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class ACharacter> Boss;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Boss", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<class AActor>> HitObjectSpawners;	// 임시로 타입은 Actor
};
