// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/GTWPlayerController.h"
#include <UI/GTWSpeedWidget.h>
#include <UI/GTWTimerWidget.h>
#include <UI/GTWPauseMenuWidget.h>

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"

AGTWPlayerController::AGTWPlayerController()
{
	// 위젯 블루프린트 애셋에서 클래스 정보 바인딩(연결).
	static ConstructorHelpers::FClassFinder<UGTWSpeedWidget> GTWHUDSpeedWidgetRef(
		TEXT("/Game/GTW/Widgets/WBP_GTWDisplaySpeed.WBP_GTWDisplaySpeed_C")
	);

	// 로드에 성공하면 클래스 설정.
	if (GTWHUDSpeedWidgetRef.Succeeded())
	{
		GTWSpeedWidgetClass = GTWHUDSpeedWidgetRef.Class;
	}

	// 위젯 블루프린트 애셋에서 클래스 정보 바인딩(연결).
	static ConstructorHelpers::FClassFinder<UGTWTimerWidget> GTWHUDTimerWidgetRef(
		TEXT("/Game/GTW/Widgets/WBP_GTWTimer.WBP_GTWTimer_C")
	);

	// 로드에 성공하면 클래스 설정.
	if (GTWHUDTimerWidgetRef.Succeeded())
	{
		GTWTimerWidgetClass = GTWHUDTimerWidgetRef.Class;
	}

	// 위젯 블루프린트 애셋에서 클래스 정보 바인딩(연결).
	static ConstructorHelpers::FClassFinder<UGTWPauseMenuWidget> GTWPauseMenuWidgetRef(
		TEXT("/Game/GTW/Widgets/WBP_PauseMenu.WBP_PauseMenu_C")
	);

	// 로드에 성공하면 클래스 설정.
	if (GTWPauseMenuWidgetRef.Succeeded())
	{
		GTWPauseMenuWidgetClass = GTWPauseMenuWidgetRef.Class;
	}

	// IA_Pause 찾기
	static ConstructorHelpers::FObjectFinder<UInputAction> PauseAsset(
		TEXT("/Game/GTW/Characters/Player/Input/InputAction/IA_GTWPause.IA_GTWPause")
	);

	if (PauseAsset.Succeeded())
	{
		PauseAction = PauseAsset.Object;
	}

}

void AGTWPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 입력 설정.
	FInputModeGameOnly GameOnlyInputMode;
	SetInputMode(GameOnlyInputMode);

	// 위젯 생성.
	GTWSpeedWidget = CreateWidget<UGTWSpeedWidget>(this, GTWSpeedWidgetClass);

	// 문제 없이 생성됐다면 화면에 추가.
	if (nullptr != GTWSpeedWidget)
	{
		GTWSpeedWidget->AddToViewport();
	}

	// 위젯 생성.
	GTWTimerWidget = CreateWidget<UGTWTimerWidget>(this, GTWTimerWidgetClass);

	// 문제 없이 생성됐다면 화면에 추가.
	if (nullptr != GTWTimerWidget)
	{
		GTWTimerWidget->AddToViewport();
	}

	// 위젯 생성.
	GTWPauseMenuWidget = CreateWidget<UGTWPauseMenuWidget>(this, GTWPauseMenuWidgetClass);

	// 문제 없이 생성됐다면 화면에 추가.
	if (nullptr != GTWPauseMenuWidget)
	{
		GTWPauseMenuWidget->AddToViewport(100);
	}

}

void AGTWPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInput =
		Cast<UEnhancedInputComponent>(InputComponent);

	if (EnhancedInput && PauseAction)
	{
		EnhancedInput->BindAction(
			PauseAction,
			ETriggerEvent::Started,
			this,
			&AGTWPlayerController::HandlePauseInput
		);
	}
}

void AGTWPlayerController::HandlePauseInput()
{
	if (bBlockPauseMenu || !IsValid(GTWPauseMenuWidget))
	{
		return;
	}

	GTWPauseMenuWidget->ToggleMenu();
}