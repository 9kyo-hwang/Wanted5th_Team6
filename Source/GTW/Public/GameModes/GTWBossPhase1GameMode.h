// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include <GameModes/GTWBossGameModeTypes.h>
#include "GTWBossPhase1GameMode.generated.h"


/**
 * 보스 1페이즈 레벨에서 사용되는 게임 모드
 * - TODO: 4층 소코반/슈팅 게임 클리어 결과에 따라 보스의 난이도가 바뀜
 * - 성공: 보스 유효타 3회 / 실패: 맵 밖으로 떨어져서 바닥에 깔린 TriggerBox에 플레이어가 닿으면
 */

/*
* 1페 보스쪽에 필요한 것
* OnValidHit: 인자 없는 C++ 멀티캐스트 Delegate
* StartCombat(): 전투 시작
* StopCombat(): 공격·예약·HitObject 정리, 반복 호출 가
*/

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIntroRequestedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOutroRequestedDelegate, EBossGameModeState, Result);

class AGTWBoss;

UCLASS()
class GTW_API AGTWBossPhase1GameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	virtual void StartPlay() override;	
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:
	// Level Blueprint에서 배치된 보스의 참조를 전달하는 통로
	UFUNCTION(BlueprintCallable, Category="Boss|Phase")
	void InitGameplay(AGTWBoss* InBoss);

	// 추후 연출이 들어온다면 연출 완료 시 Callback으로 해당 함수 호출(지금은 Level BeginPlay 시 바로 수행)
	UFUNCTION(BlueprintCallable, Category = "Boss|Phase")
	void StartGameplay();

	// Map에 깔려있는 TriggerVolume에서 overlap 판정 감지해서 호출하게 될 함수
	UFUNCTION(BlueprintCallable, Category = "Boss|Phase")
	void FailPhase();

	// 보스 인트로 요청
	UPROPERTY(BlueprintAssignable, Category = "Boss|Phase")
	FOnIntroRequestedDelegate OnIntroRequested;

	// 보스 아웃트로 요청
	UPROPERTY(BlueprintAssignable, Category = "Boss|Phase")
	FOnOutroRequestedDelegate OnOutroRequested;

	UFUNCTION(BlueprintCallable, Category = "Boss|Phase")
	void OpenNextLevel();

protected:
	// PlayerController 입력 허용 등...
	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Phase", meta=(DisplayName="On Gameplay Started"))
	void K2_OnGameplayStarted();
	
	// PlayerController 입력 블락 등...
	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Phase", meta = (DisplayName = "On Gameplay Stopped"))
	void K2_OnGameplayStopped();

private:
	void OnHitBoss();	// 보스가 유효타 맞았을 때 호출될 콜백. Boss측 OnValidHit 델리게이트가 발행될 때.
	void UnbindBossHitDelegate();	// 델리게이트 바인딩할 때 반환값을 캐싱해두고, 구독 해제

	void ClearPhase();	// 보스 유효타 기반 판정 처리
	void StopGameplay();	// 성공 또는 실패 시 맵에 있는 액터들을 중지(정리)하는 역할

private:
	static constexpr int32 ClearHitCount = 3;	// 3번 맞추면 클리어
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Boss|Phase", meta=(AllowPrivateAccess = "true"))
	EBossGameModeState CurrentState = EBossGameModeState::Ready;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Boss|Phase", meta = (AllowPrivateAccess = "true"))
	int32 CurrentHitCount = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Boss", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AGTWBoss> Boss;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Level")
	TSoftObjectPtr<UWorld> Phase1Level;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Level")
	TSoftObjectPtr<UWorld> Phase2Level;

	bool bRequestedOpenNextLevel = false;

	FDelegateHandle BossHitDelegateHandle;
};
