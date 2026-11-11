// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GTWTotalTimeWidget.h"
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
	SecondGTW = Cast<UTextBlock>(GetWidgetFromName(TEXT("Second")));
	//SecondGTW->SetText()
	FourthSokoban = Cast<UTextBlock>(GetWidgetFromName(TEXT("Sokoban")));
	//SecondGTW->SetText()
	SecondGTW = Cast<UTextBlock>(GetWidgetFromName(TEXT("Shooting")));
	//SecondGTW->SetText()
	SecondGTW = Cast<UTextBlock>(GetWidgetFromName(TEXT("FirstPhase")));
	//SecondGTW->SetText()
	SecondGTW = Cast<UTextBlock>(GetWidgetFromName(TEXT("SecondPhase")));
	//SecondGTW->SetText()


}
