// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include <GameModes/GTWBossGameModeTypes.h>
#include "GTWBaseBossGameMode.generated.h"

// PURE_VIRTUAL: UObject 생태계에서 순수 가상 함수를 놓고 싶을 때.
// 모든 UObject는 CDO가 있어야 하는(인스턴스화) 구조라 = 0으로 하지는 않고,
// 대신 Runtime에서 크래시를 발생시키도록 함

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIntroRequestedDelegate);

UCLASS(Abstract)
class GTW_API AGTWBaseBossGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AGTWBaseBossGameMode();

	UFUNCTION(BlueprintCallable, Category = "Boss|Phase")
	void InitPhase(AActor* InBoss);

	// 인트로 재생 후 호출
	UFUNCTION(BlueprintCallable, Category = "Boss|Phase")
	void BeginPhase();

	UFUNCTION(BlueprintCallable, Category = "Boss|Phase")
	void ClearPhase();

	// Fail은 각 페이즈 별 GameMode에서 정의

	// 아웃트로 재생 후 호출
	UFUNCTION(BlueprintCallable, Category = "Boss|Phase")
	void CompletePhase();

	// 보스 인트로 요청
	UPROPERTY(BlueprintAssignable, Category = "Boss|Phase")
	FOnIntroRequestedDelegate OnIntroRequested;

protected:
	virtual void StartPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	void EndPhase(EBossGameModeState Result);

	FORCEINLINE AActor* GetBoss() const { return Boss.Get(); }
	FORCEINLINE bool IsPhasePlaying() const { return CurrentState == EBossGameModeState::Play; }

	// Delegate 연결이나 추가 값 세팅 등, 자식 GameMode에서 각자 보스에 맞게 재정의
	virtual bool BindBossEvents() { return true; }
	virtual void UnbindBossEvents() {}
	virtual void HandlePhaseResult(EBossGameModeState Result) {}	// 보스 멈추고 연출 실행 전
	virtual bool RequestPhaseOutro(EBossGameModeState Result) { return false; }	// 요청 있으면 발행, 
	virtual bool HandlePhaseCompletion(EBossGameModeState Result)	
		PURE_VIRTUAL(AGTWBaseBossGameMode::HandlePhaseCompletion, return false;);	// 연출 실행도 다 끝난 시점. 성공하면 true, 설정 누락 등으로 실행 안하면 false

	// 혹시 BP에서 추가로 더 해야할 게 생기면 여기 통해서 가능
	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Phase", meta = (DisplayName = "On Phase Started"))
	void K2_OnPhaseStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Phase", meta = (DisplayName = "On Phase Ended"))
	void K2_OnPhaseEnded();

private:
	void SetIgnorePlayerInput(bool bNewInput);
	void StopBoss();

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Boss|Phase", meta = (AllowPrivateAccess = "true"))
	EBossGameModeState CurrentState = EBossGameModeState::Ready;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> Boss;

	bool bPhaseCompletionRequested = false;
};
