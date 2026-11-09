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

protected:
	virtual void StartPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Boss|State")
	void Clear();
	
	UFUNCTION(BlueprintCallable, Category = "Boss|Setup", meta=(BlueprintProtected = "true"))
	void CompleteSetup_BeforeCombat();

	UFUNCTION(BlueprintCallable, Category = "Boss|Setup", meta = (BlueprintProtected = "true"))
	void CompleteSetup_AfterCombat();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Setup", meta = (BlueprintProtected = "true", DisplayName = "Begin Setup Before Combat"))
	void BP_BeginSetup_BeforeCombat();

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Setup", meta = (BlueprintProtected = "true", DisplayName = "Begin Setup After Combat"))
	void BP_BeginSetup_AfterCombat(EBossGameModeState Result);

	void EndCombat(EBossGameModeState Result);

	FORCEINLINE AActor* GetBoss() const { return Boss.Get(); }
	virtual UClass* GetBossClass() const PURE_VIRTUAL(AGTWBaseBossGameMode::GetBossClass, return nullptr;);
	FORCEINLINE bool IsPhasePlaying() const { return CurrentState == EBossGameModeState::Play; }
	virtual bool BindBossEvents() { return true; }
	virtual void UnbindBossEvents() {}
	virtual void RecordCombatResult(EBossGameModeState Result) {}	// 연출 전 기록

private:
	bool SetupBoss();
	void BeginCombat();
	
	void NativeBeginSetup_BeforeCombat();
	void NativeBeginSetup_AfterCombat();

	void SetIgnorePlayerInput(bool bNewInput);
	void StopBoss();

	bool OpenNextLevelByResult(EBossGameModeState Result);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Level", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UWorld> FailLevel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Level", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UWorld> ClearLevel;

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Boss|Phase", meta = (AllowPrivateAccess = "true"))
	EBossGameModeState CurrentState = EBossGameModeState::Ready;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> Boss;

	bool bWaitingForBeforeCombatSetup = false;
	bool bWaitingForAfterCombatSetup = false;

	bool bPlayerInputIgnored = false;
};
