// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GTWTotalTimeWidget.h"
#include <GTWGameInstance.h>
#include "UI/GTWTimerWidget.h"
#include <Components/TextBlock.h>


UGTWTotalTimeWidget::UGTWTotalTimeWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FReply UGTWTotalTimeWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UGTWTotalTimeWidget::BindTimerToText()
{
	GameInstanceRef = GetGameInstance<UGTWGameInstance>();
	UGTWGameInstance* Ins = GameInstanceRef.Get();
	// 2층
	SecondGTW = Cast<UTextBlock>(GetWidgetFromName(TEXT("Second")));
	if (Ins->LevelClearTimes.Contains(ELevelClear::SecondFloorClear))
	{
		SecondGTW->SetText(FText::AsNumber(*Ins->LevelClearTimes.Find(ELevelClear::SecondFloorClear)));
		TotalTime += *Ins->LevelClearTimes.Find(ELevelClear::SecondFloorClear);
	}

	// 4층소코반
	FourthSokoban = Cast<UTextBlock>(GetWidgetFromName(TEXT("Sokoban")));
	if (Ins->LevelClearTimes.Contains(ELevelClear::SokobanClear))
	{
		FourthSokoban->SetText(FText::AsNumber(*Ins->LevelClearTimes.Find(ELevelClear::SokobanClear)));
	}

	// 4층 슈팅게임
	FourthShooting = Cast<UTextBlock>(GetWidgetFromName(TEXT("Shooting")));
	if (Ins->LevelClearTimes.Contains(ELevelClear::ShootingGameClear))
	{
		FourthShooting->SetText(FText::AsNumber(*Ins->LevelClearTimes.Find(ELevelClear::ShootingGameClear)));
	}

	// 6층 1페
	SixthFirstPhase = Cast<UTextBlock>(GetWidgetFromName(TEXT("FirstPhase")));
	if (Ins->LevelClearTimes.Contains(ELevelClear::BossFirstPhaseGameClear))
	{
		SixthFirstPhase->SetText(FText::AsNumber(*Ins->LevelClearTimes.Find(ELevelClear::BossFirstPhaseGameClear)));
	}

	// 6층 2페
	SixthSecondPhase = Cast<UTextBlock>(GetWidgetFromName(TEXT("SecondPhase")));
	if (Ins->LevelClearTimes.Contains(ELevelClear::BossSecondPhaseGameClear))
	{
		SixthSecondPhase->SetText(FText::AsNumber(*Ins->LevelClearTimes.Find(ELevelClear::BossSecondPhaseGameClear)));
	}

	// 총합
	TotalText = Cast<UTextBlock>(GetWidgetFromName(TEXT("SecondPhase")));
	TotalText->SetText(FText::AsNumber()));


}
