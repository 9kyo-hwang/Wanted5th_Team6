// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GTWTotalTimeWidget.h"
#include <GTWGameInstance.h>
#include <Components/EditableTextBox.h>


UGTWTotalTimeWidget::UGTWTotalTimeWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGTWTotalTimeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindTimerToText();
}

void UGTWTotalTimeWidget::BindTimerToText()
{
	GameInstanceRef = GetGameInstance<UGTWGameInstance>();
	UGTWGameInstance* Ins = GameInstanceRef.Get();

	if (!IsValid(Ins))
	{
		UE_LOG(LogTemp, Error,
			TEXT("TotalTimeWidget: GTWGameInstance를 가져오지 못했습니다."));
		return;
	}

	// 타이머 초기화(중복 호출 방지)
	TotalTime = 0;

	// 2층
	SecondGTW = Cast<UEditableTextBox>(GetWidgetFromName(TEXT("Second")));
	if (IsValid(SecondGTW))
	{
		if (Ins->LevelClearTimes.Contains(ELevelClear::SecondFloorClear))
		{
			SecondGTW->SetText(FText::AsNumber(*Ins->LevelClearTimes.Find(ELevelClear::SecondFloorClear)));
			TotalTime += *Ins->LevelClearTimes.Find(ELevelClear::SecondFloorClear);
		}
	}


	// 4층소코반
	FourthSokoban = Cast<UEditableTextBox>(GetWidgetFromName(TEXT("Sokoban")));
	if (IsValid(FourthSokoban))
	{
		if (Ins->LevelClearTimes.Contains(ELevelClear::SokobanClear))
		{
			FourthSokoban->SetText(FText::AsNumber(*Ins->LevelClearTimes.Find(ELevelClear::SokobanClear)));
			TotalTime += *Ins->LevelClearTimes.Find(ELevelClear::SokobanClear);
		}
	}


	// 4층 슈팅게임
	FourthShooting = Cast<UEditableTextBox>(GetWidgetFromName(TEXT("Shooting")));
	if (IsValid(FourthShooting))
	{
		if (Ins->LevelClearTimes.Contains(ELevelClear::ShootingGameClear))
		{
			FourthShooting->SetText(FText::AsNumber(*Ins->LevelClearTimes.Find(ELevelClear::ShootingGameClear)));
			TotalTime += *Ins->LevelClearTimes.Find(ELevelClear::ShootingGameClear);
		}
	}


	// 6층 1페
	SixthFirstPhase = Cast<UEditableTextBox>(GetWidgetFromName(TEXT("FirstPhase")));
	if (IsValid(SixthFirstPhase))
	{
		if (Ins->LevelClearTimes.Contains(ELevelClear::BossFirstPhaseGameClear))
		{
			SixthFirstPhase->SetText(FText::AsNumber(*Ins->LevelClearTimes.Find(ELevelClear::BossFirstPhaseGameClear)));
			TotalTime += *Ins->LevelClearTimes.Find(ELevelClear::BossFirstPhaseGameClear);
		}
	}


	// 6층 2페
	SixthSecondPhase = Cast<UEditableTextBox>(GetWidgetFromName(TEXT("SecondPhase")));
	if (IsValid(SixthSecondPhase))
	{
		if (Ins->LevelClearTimes.Contains(ELevelClear::BossSecondPhaseGameClear))
		{
			SixthSecondPhase->SetText(FText::AsNumber(*Ins->LevelClearTimes.Find(ELevelClear::BossSecondPhaseGameClear)));
			TotalTime += *Ins->LevelClearTimes.Find(ELevelClear::BossSecondPhaseGameClear);
		}
	}


	// 총합
	TotalText = Cast<UEditableTextBox>(GetWidgetFromName(TEXT("Total")));
	if (IsValid(TotalText))
	{
		TotalText->SetText(FText::AsNumber(TotalTime));
	}


}
