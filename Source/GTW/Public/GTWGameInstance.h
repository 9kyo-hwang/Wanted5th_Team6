// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include <Tickable.h> // FTickableGameObject를 사용하기 위해 필수
#include "GTWGameInstance.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTimerChangedDelegate, double);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnClearLevelChangedDelegate, ELevelClear, double);

UENUM()
enum class ELevelClear : uint8
{
	SecondFloorClear = 0,
	SokobanClear,
	ShootingGameClear,
	BossFirstPhaseGameClear,
	BossSecondPhaseGameClear,
};

UENUM(BlueprintType)
enum class EMinigameType : uint8
{
	Sokoban UMETA(DisplayName = "Sokoban"),
	Shooting UMETA(DisplayName = "Shooting")
};

/**
 * 각 게임 별 진행 기록 + 레벨 간 전달
 * - 이번 게임 플레이의 클리어 타임
 * - 6층 보스 1/2페 클리어 유무
 * - SaveGame 통해서 저장 요청
 */
UCLASS()
class GTW_API UGTWGameInstance : public UGameInstance, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// 기존에 클리어한 이력이 있다면 Load
	virtual void Init() override;
	
	// 이번 게임 플레이에서 사용될 데이터 초기화 및 시작 시각 기록
	// TEMP: 현재 게임 시작 UI 같은 게 없어서 코드로 호출할 마땅한 경로가 없음 -> BP 열어주기
	UFUNCTION(BlueprintCallable, Category="Game|Progress")
	void StartNewGame();

	// 현재 진행중인 게임저장
	UFUNCTION(BlueprintCallable, Category = "Game|Progress")
	void SaveProgressGame();

	// 진행했던 Level부터 게임 시작
	UFUNCTION(BlueprintCallable, Category = "Game|Progress")
	bool StartProgressGame();

	// 게임 최종 클리어 시 SaveGame에 클리어타임 기록
	void CompleteGame();

	// --- FTickableGameObject 인터페이스 오버라이드 ---
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }

	FOnTimerChangedDelegate OnTimeChanged;
	FOnClearLevelChangedDelegate OnClearLevelChanged;
	void RecordLevelClear(ELevelClear Level);
	void RecordMiniGameClear(ELevelClear Level, double ClearTime);
	void InitializeTimerWidget(class UGTWTimerWidget* Widget) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsTickable = false;
	bool SetMinigameClearTime(EMinigameType Type, double InClearTimeSeconds);
	bool TryGetMinigameClearTime(EMinigameType Type, double& OutClearTimeSeconds) const;
	
	UFUNCTION(BlueprintCallable, Category = "Game|Progress")
	double GetPlaytimeSeconds() const;

	FORCEINLINE bool IsBossPhase1Cleared() const { return bIsBossPhase1Cleared; }
	FORCEINLINE void SetBossPhase1Cleared() { bIsBossPhase1Cleared = true; }
	FORCEINLINE void ResetBossPhase1Cleared() { bIsBossPhase1Cleared = false; }

private:
	void InitClearHistory();
	bool SaveClearHistory();
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	double SokobanClearTime = -1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	double ShootingClearTime = -1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	uint8 bIsBossPhase1Cleared : 1;

	double GameStartTimeSeconds = -1;
	double GameClearTimeSeconds = -1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UGTWClearHistorySaveGame> ClearHistorySaveGame;
	UPROPERTY()
	TObjectPtr<class UGTWProgressLevelSaveGame> ProgressSaveGame;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Time", meta = (AllowPrivateAccess = "true"))
	double TotalPlayTime = 0.0;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Time", meta = (AllowPrivateAccess = "true"))
	double CurrentFloorTime = 0.0;
	TMap<ELevelClear, double> LevelClearTimes;
};
