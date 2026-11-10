// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GTWTimerWidget.h"
#include <Components/HorizontalBox.h>
#include <Components/TextBlock.h>
#include "GTWGameInstance.h"

UGTWTimerWidget::UGTWTimerWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UGTWTimerWidget::UpdateTimer(double NewTime)
{
    int HourTensDigit = NewTime / 36000;
    int HourOnesDigit = static_cast<int>(NewTime) / 3600 % 10;

    int MinuteTensDigit = (static_cast<int>(NewTime) % 3600) / 600;
    int MinuteOnesDigit = ((static_cast<int>(NewTime) % 3600) / 60) % 10;

    int SecondTensDigit = (static_cast<int>(NewTime) % 60) / 10;
    int SecondOnesDigit = (static_cast<int>(NewTime) % 60) % 10;

    if (TimerText.IsValidIndex(5))
    {
        TimerText[0].Get()->SetText(FText::FromString(FString::FromInt(HourTensDigit)));
        TimerText[1].Get()->SetText(FText::FromString(FString::FromInt(HourOnesDigit)));
        TimerText[2].Get()->SetText(FText::FromString(FString::FromInt(MinuteTensDigit)));
        TimerText[3].Get()->SetText(FText::FromString(FString::FromInt(MinuteOnesDigit)));
        TimerText[4].Get()->SetText(FText::FromString(FString::FromInt(SecondTensDigit)));
        TimerText[5].Get()->SetText(FText::FromString(FString::FromInt(SecondOnesDigit)));
    }

}

void UGTWTimerWidget::UpdateClearLevel(ELevelClear Level, double ClearTime)
{

    TObjectPtr<UHorizontalBox> SelectLevelBox;
    TObjectPtr<UTextBlock> SelectLevelText;

    switch (Level)
    {
    case ELevelClear::SecondFloorClear:     
        SelectLevelBox = SFBox;
        SelectLevelText = SFText;
        break;
    case ELevelClear::SokobanClear:           
        SelectLevelBox = SBox;
        SelectLevelText = SokobanText;
        break;
    case ELevelClear::ShootingGameClear:      
        SelectLevelBox = ShtBox;
        SelectLevelText = ShootingText;
        break;
    case ELevelClear::BossFirstPhaseGameClear:   
        SelectLevelBox = Boss1Box;
        SelectLevelText = Boss1Text;
        break;
    case ELevelClear::BossSecondPhaseGameClear: 
        SelectLevelBox = Boss2Box;
        SelectLevelText = Boss2Text;
        break;
    };

    int HourTensDigit = ClearTime / 36000;
    int HourOnesDigit = static_cast<int>(ClearTime) / 3600 % 10;
    
    int MinuteTensDigit = (static_cast<int>(ClearTime) % 3600) / 600;
    int MinuteOnesDigit = ((static_cast<int>(ClearTime) % 3600) / 60) % 10;

    int SecondTensDigit = (static_cast<int>(ClearTime) % 60) / 10;
    int SecondOnesDigit = (static_cast<int>(ClearTime) % 60) % 10;

    ClearTimerText.Reset();

    for (UWidget* Child : SelectLevelBox->GetAllChildren())
    {
        if (UTextBlock* Txt = Cast<UTextBlock>(Child))
        {
            ensure(Txt);

            if (Txt->GetText().ToString() == TEXT(":"))
            {
                continue;
            }
            ClearTimerText.Add(Txt);
        }
    }

    FSlateColor ActiveTxtColor = FSlateColor(
        FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)
    );
    for (int ix = 0; ix < 6; ++ix)
    {
        ClearTimerText[ix].Get()->SetColorAndOpacity(ActiveTxtColor);
    }

    SelectLevelText.Get()->SetColorAndOpacity(ActiveTxtColor);

    if (ClearTimerText.IsValidIndex(5))
    {
        ClearTimerText[0].Get()->SetText(FText::FromString(FString::FromInt(HourTensDigit)));
        ClearTimerText[1].Get()->SetText(FText::FromString(FString::FromInt(HourOnesDigit)));
        ClearTimerText[2].Get()->SetText(FText::FromString(FString::FromInt(MinuteTensDigit)));
        ClearTimerText[3].Get()->SetText(FText::FromString(FString::FromInt(MinuteOnesDigit)));
        ClearTimerText[4].Get()->SetText(FText::FromString(FString::FromInt(SecondTensDigit)));
        ClearTimerText[5].Get()->SetText(FText::FromString(FString::FromInt(SecondOnesDigit)));
    }

}

void UGTWTimerWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UGTWGameInstance* GameInstance = GetGameInstance<UGTWGameInstance>())
    {
        TimerGameInstance = GameInstance;
        GameInstance->OnTimeChanged.RemoveAll(this);
        GameInstance->OnClearLevelChanged.RemoveAll(this);
        GameInstance->OnTimeChanged.AddUObject(this, &UGTWTimerWidget::UpdateTimer);
        GameInstance->OnClearLevelChanged.AddUObject(this, &UGTWTimerWidget::UpdateClearLevel);
        GameInstance->InitializeTimerWidget(this);
    }
}

void UGTWTimerWidget::NativeDestruct()
{
    if (UGTWGameInstance* GameInstance = TimerGameInstance.Get())
    {
        GameInstance->OnTimeChanged.RemoveAll(this);
        GameInstance->OnClearLevelChanged.RemoveAll(this);
    }
    TimerGameInstance.Reset();
    Super::NativeDestruct();
}

void UGTWTimerWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    
    TimerText.Reset();

    for (UWidget* Child : TotalBox->GetAllChildren())
    {
        if (UTextBlock* Txt = Cast<UTextBlock>(Child))
        {
            ensure(Txt);
            
            if (Txt->GetText().ToString() == TEXT(":"))
            {
                continue;
            }

            TimerText.Add(Txt);
        }
    }
}
