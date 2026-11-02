// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/GTWPlayerController.h"
#include <UI/GTWSpeedWidget.h>

AGTWPlayerController::AGTWPlayerController()
{
	// 위젯 블루프린트 애셋에서 클래스 정보 바인딩(연결).
	static ConstructorHelpers::FClassFinder<UGTWSpeedWidget> ABHUDWidgetRef(
		TEXT("/Game/GTW/Widgets/WBP_GTWDisplaySpeed.WBP_GTWDisplaySpeed_C")
	);

	// 로드에 성공하면 클래스 설정.
	if (ABHUDWidgetRef.Succeeded())
	{
		GTWSpeedWidgetClass = ABHUDWidgetRef.Class;
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

}
