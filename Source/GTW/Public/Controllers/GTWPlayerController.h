// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GTWPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class GTW_API AGTWPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AGTWPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;


protected:
	// 속도 위젯 클래스 정보.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HUD)
	TSubclassOf<class UGTWSpeedWidget> GTWSpeedWidgetClass;

	// 생성한 속도 위젯을 저장할 변수.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = HUD)
	TObjectPtr<UGTWSpeedWidget> GTWSpeedWidget;

	// 시간 위젯 클래스 정보.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HUD)
	TSubclassOf<class UGTWTimerWidget> GTWTimerWidgetClass;

	// 생성한 시간 위젯을 저장할 변수.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = HUD)
	TObjectPtr<UGTWTimerWidget> GTWTimerWidget;

	// 메뉴 위젯 클래스 정보.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HUD)
	TSubclassOf<class UGTWPauseMenuWidget> GTWPauseMenuWidgetClass;

	// 생성한 메뉴 위젯을 저장할 변수.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = HUD)
	TObjectPtr<UGTWPauseMenuWidget> GTWPauseMenuWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> PlayerMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputAction> PauseAction;


private:
	void HandlePauseInput();
};
