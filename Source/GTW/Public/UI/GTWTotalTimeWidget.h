// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GTWTotalTimeWidget.generated.h"

enum class ELevelClear : uint8;
class UTextBlock;
/**
 * 
 */
UCLASS()
class GTW_API UGTWTotalTimeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UGTWTotalTimeWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

private:
	// TextBlock에 시간 기록을 바인딩하는 함수. 
	UFUNCTION()
	void BindTimerToText();

	void SetTimerText(ELevelClear ClearLevel,  double ClearTime);

private:
	//// 2층
	//UPROPERTY()
	//TObjectPtr<class UEditableTextBox> SecondGTW;

	//// 4층: 소코반
	//UPROPERTY()
	//TObjectPtr<class UEditableTextBox> FourthSokoban;

	//// 4층: 슈팅게임
	//UPROPERTY()
	//TObjectPtr<class UEditableTextBox> FourthShooting;

	//// 6층: 1페이즈
	//UPROPERTY()
	//TObjectPtr<class UEditableTextBox> SixthFirstPhase;

	//// 6층: 2페이즈
	//UPROPERTY()
	//TObjectPtr<class UEditableTextBox> SixthSecondPhase;

	//// 총합
	//UPROPERTY()
	//TObjectPtr<class UEditableTextBox> TotalText;

	// 게임인스턴스 가져오기
	UPROPERTY()
	TWeakObjectPtr<class UGTWGameInstance> GameInstanceRef;

	// 2층 시간
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHorizontalBox> TotalSFBox;

	// 소코반 박스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> TotalSBox;

	// 슈팅게임 박스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> TotalShtBox;

	// 6층 1페 박스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> TotalBoss1Box;

	// 6층 2페 박스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> TotalBoss2Box;

	// 총합 박스 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> TotalTimeBox;

	// 클리어 시간 기록 텍스트 박스
	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> TotalClearTimerText;

};
