// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GTWPlayerController.generated.h"

class UUserWidget;

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

public:
	UFUNCTION(BlueprintCallable, Category="UI|Input")
	void SetResumeInputWidget(UUserWidget* InWidget);

	UFUNCTION(BlueprintCallable, Category = "UI|Input")
	void ClearResumeInputWidget(UUserWidget* InWidget);

	UFUNCTION(BlueprintPure, Category = "UI|Input")
	bool IsPauseMenuOpen();

	void RestoreInputMode();

public:
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	bool bBlockPauseMenu = false;

private:
	void HandlePauseInput();

protected:

#pragma region [Widget]
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

	// 메뉴 종료 후 입력 다시 받을 기존 위젯
	UPROPERTY(Transient)
	TWeakObjectPtr<class UUserWidget> ResumeInputWidget;

#pragma endregion

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> PlayerMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputAction> PauseAction;
};
