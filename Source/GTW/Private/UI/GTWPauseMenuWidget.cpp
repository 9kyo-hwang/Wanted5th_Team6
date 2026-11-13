// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GTWPauseMenuWidget.h"
#include <Interfaces/GTWWidgetInterface.h>
#include <GTWGameInstance.h>
#include <Controllers/GTWPlayerController.h>

#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"
#include <Components/CanvasPanel.h>
#include <Components/Button.h>
#include "Components/Slider.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Kismet/GameplayStatics.h"



UGTWPauseMenuWidget::UGTWPauseMenuWidget(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{
    bIsVisiblePauseMenu = false;
    bIsVisibleSettingMenu = false;
}

void UGTWPauseMenuWidget::UnfocusMenu(bool bIsActive)
{
    AGTWPlayerController* PlayerController = Cast<AGTWPlayerController>(GetOwningPlayer());
    if (!IsValid(PlayerController))
    {
        return;
    }

    // 비활성화 시 메뉴 자동 닫기
    if (!bIsActive && !bIsVisiblePauseMenu && !PlayerController->bBlockPauseMenu)
    {
        ToggleMenu();
    }
}

bool UGTWPauseMenuWidget::ToggleMenu()
{
    AGTWPlayerController* PlayerController = Cast<AGTWPlayerController>(GetOwningPlayer());

    if (!IsValid(PlayerController))
    {
        return bIsVisiblePauseMenu;
    }

    if (!IsValid(SettingMenu))
    {
        return bIsVisiblePauseMenu;
    }

    // 메뉴 창이 켜져 있을 때
    if (bIsVisiblePauseMenu)
    {
        // 닫기 확정
        bIsVisiblePauseMenu = false;

        // Pause/Settig 메뉴 숨기기
        PauseMenu.Get()->SetVisibility(ESlateVisibility::Collapsed);
        if (IsValid(SettingMenu))
        {
            SettingMenu->SetVisibility(ESlateVisibility::Collapsed);
            bIsVisibleSettingMenu = false;
        }

        // 게임 재개
        PlayerController->SetPause(false);
        
        // 시간 측정 재개
        UGTWGameInstance* GTWGameInst = GetGameInstance<UGTWGameInstance>();
        GTWGameInst->bIsTickable = true;

        PlayerController->bShowMouseCursor = false;
        PlayerController->RestoreInputMode();
    }
    // 메뉴 창이 꺼져 있을 때
    else
    {
        // 열기 확정
        bIsVisiblePauseMenu = true;

        // Pause 메뉴 표시
        PauseMenu.Get()->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

        // 게임 일시정지
        PlayerController->SetPause(true);

        // 시간 측정 재개
        UGTWGameInstance* GTWGameInst = GetGameInstance<UGTWGameInstance>();
        GTWGameInst->bIsTickable = false;

        PlayerController->bShowMouseCursor = true;

        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        InputMode.SetHideCursorDuringCapture(false);

        PlayerController->SetInputMode(InputMode);
        SetUserFocus(PlayerController);
    }

    return bIsVisiblePauseMenu;
}

void UGTWPauseMenuWidget::GoToTitle()
{
    // 타이틀로 돌아가는 로직
    UGTWGameInstance* GTWGameInst = GetGameInstance<UGTWGameInstance>();
    GTWGameInst->OpenGameLevel("Title");
}

void UGTWPauseMenuWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // 포커스를 잃었을 때 기능을 사용하기 위한 이닛
    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication::Get()
            .OnApplicationActivationStateChanged()
            .AddUObject(
                this,
                &UGTWPauseMenuWidget::UnfocusMenu
            );
    }

    ProgressSaveGameInstance =
        Cast<UGTWGameInstance>(GetGameInstance());

    // 버튼에 델리게이트 등록
    if (ResumeGame)
    {
        ResumeGame->OnClicked.AddDynamic(
            this,
            &UGTWPauseMenuWidget::ResumeClicked
        );
    }

    if (Setting)
    {
        Setting->OnClicked.AddDynamic(
            this,
            &UGTWPauseMenuWidget::SettingClicked
        );
    }

    if (Title)
    {
        Title->OnClicked.AddDynamic(
            this,
            &UGTWPauseMenuWidget::TitleClicked
        );
    }

    // 볼륨 델리게이트 등록
    BGMSlider->OnValueChanged.AddDynamic(
        this,
        &UGTWPauseMenuWidget::OnBGMVolumeChanged
    );

    SFXSlider->OnValueChanged.AddDynamic(
        this,
        &UGTWPauseMenuWidget::OnSFXVolumeChanged
    );
}

void UGTWPauseMenuWidget::ResumeClicked()
{
    ToggleMenu();
}

void UGTWPauseMenuWidget::SettingClicked()
{
    if (!IsValid(SettingMenu))
    {
        return;
    }
    if (!bIsVisibleSettingMenu)
    {
        SettingMenu->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        bIsVisibleSettingMenu = true;
    }
    else
    {
        SettingMenu->SetVisibility(ESlateVisibility::Collapsed);
        bIsVisibleSettingMenu = false;
    }
    
}

void UGTWPauseMenuWidget::TitleClicked()
{
    GoToTitle();
}

void UGTWPauseMenuWidget::OnBGMVolumeChanged(float Value)
{
    if (UGTWGameInstance* GameInst =
        Cast<UGTWGameInstance>(GetGameInstance()))
    {
        GameInst->SetBGMVolume(Value);
    }
}

void UGTWPauseMenuWidget::OnSFXVolumeChanged(float Value)
{
    if (UGTWGameInstance* GameInst =
        Cast<UGTWGameInstance>(GetGameInstance()))
    {
        GameInst->SetSFXVolume(Value);
    }
}
