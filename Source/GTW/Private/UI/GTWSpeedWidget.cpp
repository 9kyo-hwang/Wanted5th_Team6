// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GTWSpeedWidget.h"
#include <Components/HorizontalBox.h>
#include <Components/Image.h>
#include <Components/TextBlock.h>
#include <Interfaces/GTWWidgetInterface.h>

UGTWSpeedWidget::UGTWSpeedWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UGTWSpeedWidget::UpdateSpeed(int Speed)
{
    int DigitFlag = 0;

    if (Speed >= 100)
    {
        DigitFlag = 3;
    }
    else if (Speed >= 10)
    {
        DigitFlag = 2;
    }
    else if (Speed > 0)
    {
        DigitFlag = 1;
    }
    else
    {
        DigitFlag = 0;
    }

    int HundredDigit = Speed / 100;
    int TensDigit = Speed % 100 / 10;
    int OnesDigit = Speed % 10;

    SpeedText[0].Get()->SetText(FText::FromString(FString::FromInt(HundredDigit)));
    SpeedText[1].Get()->SetText(FText::FromString(FString::FromInt(TensDigit)));
    SpeedText[2].Get()->SetText(FText::FromString(FString::FromInt(OnesDigit)));

    
    FSlateColor ActiveTxtColor = FSlateColor(
        FLinearColor(1.0f , 1.0f, 1.0f, 1.0f)
    );
    for (int ix = 2; ix >= 3 - DigitFlag;--ix)
    {
        SpeedText[ix].Get()->SetColorAndOpacity(ActiveTxtColor);
    }
    FSlateColor InActiveTxtColor = FSlateColor(
        FLinearColor(1.0f, 1.0f, 1.0f, 0.7f)
    );
    for (int ix = 2 - DigitFlag; ix >= 0;--ix)
    {
        SpeedText[ix].Get()->SetColorAndOpacity(InActiveTxtColor);
    }
    

    UpdateSpeedGauge(Speed);
}

void UGTWSpeedWidget::UpdateSpeedGauge(int Speed)
{
    int ActiveGaugeSlots = Speed / 15;

    FSlateColor ActiveColor = FSlateColor(
        FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)
    );
    FSlateColor InActiveColor = FSlateColor(
        FLinearColor(1.0f, 1.0f, 1.0f, 0.5f)
    );


    for (int ix = 0; ix < 10; ++ix)
    {
        if (ix > 5)
        {
            ActiveColor = FSlateColor(
                FLinearColor(1.0f, 0.5f, 0.0f, 1.0f)
            );
        }
        if (ix > 7)
        {
            ActiveColor = FSlateColor(
                FLinearColor(1.0f, 0.0f, 0.0f, 1.0f)
            );
        }

        if (ix < ActiveGaugeSlots)
        {
            SpeedGauges[ix]->SetBrushTintColor(ActiveColor);
        }
        else
        {
            SpeedGauges[ix]->SetBrushTintColor(InActiveColor);
        }
    }
}


void UGTWSpeedWidget::NativeConstruct()
{
	Super::NativeConstruct();

    // 세팅
    IGTWWidgetInterface* HUDPawn = GetOwningPlayerPawn<IGTWWidgetInterface>();
    if (nullptr != HUDPawn)
    {
        HUDPawn->SetSpeedWidget(this);
    }

}

void UGTWSpeedWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    
    SpeedGauges.Reset();

    for (UWidget* Child : SpeedGaugeBox->GetAllChildren())
    {
        if (UImage* Image = Cast<UImage>(Child))
        {
            ensure(Image);

            SpeedGauges.Add(Image);
        }
    }

    SpeedText.Reset();

    for (UWidget* Child : SpeedBox->GetAllChildren())
    {
        if (UTextBlock* Txt = Cast<UTextBlock>(Child))
        {
            ensure(Txt);

            SpeedText.Add(Txt);
        }
    }
}
