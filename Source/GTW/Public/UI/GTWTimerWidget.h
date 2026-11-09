// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GTWTimerWidget.generated.h"

// 레벨 클리어를 알릴 열거형
UENUM()
enum class ELevelClear : uint8
{
	SecondFloorClear = 0,
	SokobanClear,
	ShootingGameClear,
	BossFirstPhaseGameClear,
	BossSecondPhaseGameClear,
};

/**
 * 
 */
UCLASS()
class GTW_API UGTWTimerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UGTWTimerWidget(const FObjectInitializer& ObjectInitializer);

	void UpdateTimer(double NewTime);

	void UpdateClearLevel(ELevelClear Level, double ClearTime);

protected:
	virtual void NativeConstruct() override;

	virtual void NativeOnInitialized() override;

protected:
	// 위젯을 소유하는 액터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Actor)
	TObjectPtr<AActor> OwningActor;

	// 전체 시간
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHorizontalBox> TotalBox;

	// 시간 기록 텍스트 박스
	UPROPERTY(Transient)
	TArray<TObjectPtr<class UTextBlock>> TimerText;

	// 2층 시간
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> SFBox;

	// 소코반 시간
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> SBox;

	// 슈팅 게임 시간
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> ShtBox;

	// 보스 1페이지 시간
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> Boss1Box;

	// 보스 2페이지 시간
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> Boss2Box;

	// 2층 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SFText;

	// 소코반 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SokobanText;

	// 슈팅 게임 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ShootingText;

	// 보스 1페이지 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Boss1Text;

	// 보스 2페이지 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Boss2Text;

	// 클리어 시간 기록 텍스트 박스
	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> ClearTimerText;

	// 클리어 기록 여부
	UPROPERTY()
	bool ClearSF;

	UPROPERTY()
	bool ClearSokoban;

	UPROPERTY()
	bool ClearShooting;

	UPROPERTY()
	bool ClearBoss1;

	UPROPERTY()
	bool ClearBoss2;

};
