// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GTWTotalTimeWidget.h"
#include <GTWGameInstance.h>
#include <Components/EditableTextBox.h>

#include <Components/HorizontalBox.h>
#include <Components/TextBlock.h>



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

	UE_LOG(LogTemp, Log, TEXT("BindTimerToText 함수호출됨"));

	GameInstanceRef = GetGameInstance<UGTWGameInstance>();
	UGTWGameInstance* Ins = GameInstanceRef.Get();

	if (!IsValid(Ins))
	{
		UE_LOG(LogTemp, Error,
			TEXT("TotalTimeWidget: GTWGameInstance를 가져오지 못했습니다."));
		return;
	}

	// 2층
	//SecondGTW = Cast<UEditableTextBox>(GetWidgetFromName(TEXT("Second")));
	if (IsValid(TotalSFBox))
	{
		if (Ins->LevelClearTimes.Contains(ELevelClear::SecondFloorClear))
		{
			double NewSecondClearTime = *Ins->LevelClearTimes.Find(ELevelClear::SecondFloorClear);
			SetTimerText(ELevelClear::SecondFloorClear, NewSecondClearTime);

		}
	}


	// 4층소코반
	//FourthSokoban = Cast<UEditableTextBox>(GetWidgetFromName(TEXT("Sokoban")));
	if (IsValid(TotalSBox))
	{
		if (Ins->LevelClearTimes.Contains(ELevelClear::SokobanClear))
		{
			double NewSokobanClearTime = *Ins->LevelClearTimes.Find(ELevelClear::SokobanClear);
			SetTimerText(ELevelClear::SokobanClear, NewSokobanClearTime);
		}
	}


	// 4층 슈팅게임
	//FourthShooting = Cast<UEditableTextBox>(GetWidgetFromName(TEXT("Shooting")));
	if (IsValid(TotalShtBox))
	{
		if (Ins->LevelClearTimes.Contains(ELevelClear::ShootingGameClear))
		{
			double NewShootingClearTime = *Ins->LevelClearTimes.Find(ELevelClear::ShootingGameClear);
			SetTimerText(ELevelClear::ShootingGameClear, NewShootingClearTime);
		}
	}


	// 6층 1페
	//SixthFirstPhase = Cast<UEditableTextBox>(GetWidgetFromName(TEXT("FirstPhase")));
	if (IsValid(TotalBoss1Box))
	{
		if (Ins->LevelClearTimes.Contains(ELevelClear::BossFirstPhaseGameClear))
		{
			double NewBoss1ClearTime = *Ins->LevelClearTimes.Find(ELevelClear::BossFirstPhaseGameClear);
			SetTimerText(ELevelClear::BossFirstPhaseGameClear, NewBoss1ClearTime);
		}
	}


	// 6층 2페
	//SixthSecondPhase = Cast<UEditableTextBox>(GetWidgetFromName(TEXT("SecondPhase")));
	if (IsValid(TotalBoss2Box))
	{
		if (Ins->LevelClearTimes.Contains(ELevelClear::BossSecondPhaseGameClear))
		{
			double NewBoss2ClearTime = *Ins->LevelClearTimes.Find(ELevelClear::BossSecondPhaseGameClear);
			SetTimerText(ELevelClear::BossSecondPhaseGameClear, NewBoss2ClearTime);
		}
	}


	// 총합
	//TotalText = Cast<UEditableTextBox>(GetWidgetFromName(TEXT("Total")));
	if (IsValid(TotalTimeBox))
	{
		double NewTotalTime = Ins->GetTotalTime();

		int HourTensDigit = static_cast<int>(NewTotalTime) / 36000;
		int HourOnesDigit = static_cast<int>(NewTotalTime) / 3600 % 10;

		int MinuteTensDigit = (static_cast<int>(NewTotalTime) % 3600) / 600;
		int MinuteOnesDigit = ((static_cast<int>(NewTotalTime) % 3600) / 60) % 10;

		int SecondTensDigit = (static_cast<int>(NewTotalTime) % 60) / 10;
		int SecondOnesDigit = (static_cast<int>(NewTotalTime) % 60) % 10;

		TotalClearTimerText.Reset();

		for (UWidget* Child : TotalTimeBox->GetAllChildren())
		{
			if (UTextBlock* Txt = Cast<UTextBlock>(Child))
			{
				ensure(Txt);

				if (Txt->GetText().ToString() == TEXT(":"))
				{
					continue;
				}
				TotalClearTimerText.Add(Txt);
			}
		}

		if (TotalClearTimerText.IsValidIndex(5))
		{
			TotalClearTimerText[0].Get()->SetText(FText::FromString(FString::FromInt(HourTensDigit)));
			TotalClearTimerText[1].Get()->SetText(FText::FromString(FString::FromInt(HourOnesDigit)));
			TotalClearTimerText[2].Get()->SetText(FText::FromString(FString::FromInt(MinuteTensDigit)));
			TotalClearTimerText[3].Get()->SetText(FText::FromString(FString::FromInt(MinuteOnesDigit)));
			TotalClearTimerText[4].Get()->SetText(FText::FromString(FString::FromInt(SecondTensDigit)));
			TotalClearTimerText[5].Get()->SetText(FText::FromString(FString::FromInt(SecondOnesDigit)));
		}
	}


}

void UGTWTotalTimeWidget::SetTimerText(ELevelClear ClearLevel, double ClearTime)
{
	TObjectPtr<UHorizontalBox> SelectLevelBox;
	TObjectPtr<UTextBlock> SelectLevelText;


	UE_LOG(LogTemp, Log, TEXT("ClearTime: %d"), static_cast<int>(ClearTime));

	int HourTensDigit = static_cast<int>(ClearTime) / 36000;
	int HourOnesDigit = static_cast<int>(ClearTime) / 3600 % 10;

	int MinuteTensDigit = (static_cast<int>(ClearTime) % 3600) / 600;
	int MinuteOnesDigit = ((static_cast<int>(ClearTime) % 3600) / 60) % 10;

	int SecondTensDigit = (static_cast<int>(ClearTime) % 60) / 10;
	int SecondOnesDigit = (static_cast<int>(ClearTime) % 60) % 10;

	switch (ClearLevel)
	{
	case ELevelClear::SecondFloorClear:
		SelectLevelBox = TotalSFBox;
		break;
	case ELevelClear::SokobanClear:
		SelectLevelBox = TotalSBox;
		break;
	case ELevelClear::ShootingGameClear:
		SelectLevelBox = TotalShtBox;
		break;
	case ELevelClear::BossFirstPhaseGameClear:
		SelectLevelBox = TotalBoss1Box;
		break;
	case ELevelClear::BossSecondPhaseGameClear:
		SelectLevelBox = TotalBoss2Box;
		break;
	};

	if (!IsValid(SelectLevelBox.Get()))
	{
		UE_LOG(LogTemp, Error,
			TEXT("박스 바인딩 실패: ClearLevel=%d, "
				"SF=%s, Sokoban=%s, Shooting=%s, Boss1=%s, Boss2=%s"),
			static_cast<int32>(ClearLevel),
			*GetNameSafe(TotalSFBox.Get()),
			*GetNameSafe(TotalSBox.Get()),
			*GetNameSafe(TotalShtBox.Get()),
			*GetNameSafe(TotalBoss1Box.Get()),
			*GetNameSafe(TotalBoss2Box.Get()));
		return;
	}


	TotalClearTimerText.Reset();

	for (UWidget* Child : SelectLevelBox->GetAllChildren())
	{
		if (UTextBlock* Txt = Cast<UTextBlock>(Child))
		{
			ensure(Txt);

			if (Txt->GetText().ToString() == TEXT(":"))
			{
				continue;
			}
			TotalClearTimerText.Add(Txt);
		}
	}


	if (TotalClearTimerText.IsValidIndex(5))
	{
		TotalClearTimerText[0].Get()->SetText(FText::FromString(FString::FromInt(HourTensDigit)));
		TotalClearTimerText[1].Get()->SetText(FText::FromString(FString::FromInt(HourOnesDigit)));
		TotalClearTimerText[2].Get()->SetText(FText::FromString(FString::FromInt(MinuteTensDigit)));
		TotalClearTimerText[3].Get()->SetText(FText::FromString(FString::FromInt(MinuteOnesDigit)));
		TotalClearTimerText[4].Get()->SetText(FText::FromString(FString::FromInt(SecondTensDigit)));
		TotalClearTimerText[5].Get()->SetText(FText::FromString(FString::FromInt(SecondOnesDigit)));
	}
}
