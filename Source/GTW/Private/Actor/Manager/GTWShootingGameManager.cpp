// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Manager/GTWShootingGameManager.h"
#include <Kismet/GameplayStatics.h>
#include <Blueprint/UserWidget.h>
#include <GameFramework/PlayerController.h>

// Sets default values
AGTWShootingGameManager::AGTWShootingGameManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Player = UGameplayStatics::GetPlayerController(this, 0);
}

// Called when the game starts or when spawned
void AGTWShootingGameManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGTWShootingGameManager::ShowElapsedWidget()
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

void AGTWShootingGameManager::ShowClearViewWidget()
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

// Called every frame
void AGTWShootingGameManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsGameplayTimerRunning)
	{
		ElapsedTime += DeltaTime;
	}
}

void AGTWShootingGameManager::Reset()
{
	ShowElapsedWidget();
}

void AGTWShootingGameManager::GameClear()
{
	bIsGameplayTimerRunning = true;
	ShowClearViewWidget();
	UE_LOG(LogTemp, Warning, TEXT("ElapsedTiem : %d"), int(ElapsedTime));
}

