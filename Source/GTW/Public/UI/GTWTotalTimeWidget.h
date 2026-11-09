// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GTWTotalTimeWidget.generated.h"

/**
 * 
 */
UCLASS()
class GTW_API UGTWTotalTimeWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UGTWTotalTimeWidget() = default;

	virtual FReply NativeOnKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent
	) override;

private:
	// TextBlock에 시간 기록을 바인딩하는 함수. 
	UFUNCTION()
	void BindTimerToText();

private:
	// 2층
	UPROPERTY()
	TObjectPtr<class UTextBlock> SecondGTW;

	// 4층: 소코반
	UPROPERTY()
	TObjectPtr<class UTextBlock> FourthSokoban;

	// 4층: 슈팅게임
	UPROPERTY()
	TObjectPtr<class UTextBlock> FourthShooting;

	// 6층: 1페이즈
	UPROPERTY()
	TObjectPtr<class UTextBlock> SixthFirstPhase;

	// 6층: 2페이즈
	UPROPERTY()
	TObjectPtr<class UTextBlock> SixthSecondPhase;

	// 총합
	UPROPERTY()
	float TotalTime;
};
