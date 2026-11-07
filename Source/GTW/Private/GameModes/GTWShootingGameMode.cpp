// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/GTWShootingGameMode.h"
#include <Kismet/GameplayStatics.h>
#include <Blueprint/UserWidget.h>
#include <GameFramework/PlayerController.h>

AGTWShootingGameMode::AGTWShootingGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	
}

void AGTWShootingGameMode::BeginPlay()
{
	Super::BeginPlay();

	Player = UGameplayStatics::GetPlayerController(this, 0);
}

void AGTWShootingGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsGameplayTimerRunning)
	{
		ElapsedTime += DeltaSeconds;
	}
}

void AGTWShootingGameMode::Reset()
{
	ShowElapsedWidget();
}

void AGTWShootingGameMode::GameClear()
{
	bIsGameplayTimerRunning = true;
	ShowClearViewWidget();
	UE_LOG(LogTemp, Warning, TEXT("ElapsedTiem : %d"), int(ElapsedTime));
}

void AGTWShootingGameMode::ShowElapsedWidget()
{
	ElapsedWidget = CreateWidget<UUserWidget>(
		Player,
		ElapsedWidgetClass
	);

	if (IsValid(ElapsedWidget))
	{
		ElapsedWidget->AddToViewport();
		ElapsedWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void AGTWShootingGameMode::ShowClearViewWidget()
{
	ClearViewWidget = CreateWidget<UUserWidget>(
		Player,
		ClearViewWidgetClass
	);

	if (IsValid(ClearViewWidget))
	{
		ClearViewWidget->AddToViewport();
		ClearViewWidget->SetVisibility(ESlateVisibility::Visible);
	}
}
