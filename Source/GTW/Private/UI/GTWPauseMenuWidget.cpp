// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GTWPauseMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"
#include <Interfaces/GTWWidgetInterface.h>
#include <Components/CanvasPanel.h>
#include <Components/Button.h>
#include "Kismet/GameplayStatics.h"


UGTWPauseMenuWidget::UGTWPauseMenuWidget(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{
    bIsVisiblePauseMenu = false;
    bIsVisibleSettingMenu = false;
}

void UGTWPauseMenuWidget::UnfocusMenu(bool bIsActive)
{
    if (!bIsVisiblePauseMenu)
    {
        ToggleMenu();
    }

}

bool UGTWPauseMenuWidget::ToggleMenu()
{
    APlayerController* PlayerController = GetOwningPlayer();

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
        PauseMenu.Get()->SetVisibility(ESlateVisibility::Collapsed);
        if (IsValid(SettingMenu))
        {
            SettingMenu->SetVisibility(ESlateVisibility::Collapsed);
            bIsVisibleSettingMenu = false;
        }

        // 게임 재개
        PlayerController->SetPause(false);

        // 마우스 커서 숨기기
        PlayerController->bShowMouseCursor = false;

        // 게임 입력으로 복구
        FInputModeGameOnly InputMode;
        PlayerController->SetInputMode(InputMode);
    }
    // 메뉴 창이 꺼져 있을 때
    else
    {
        PauseMenu.Get()->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

        // 게임 일시정지
        PlayerController->SetPause(true);

        // 마우스 커서 표시
        PlayerController->bShowMouseCursor = true;

        // UI 조작 허용
        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        InputMode.SetHideCursorDuringCapture(false);
        PlayerController->SetInputMode(InputMode);
    }
    bIsVisiblePauseMenu = !bIsVisiblePauseMenu;

    return bIsVisiblePauseMenu;

}


void UGTWPauseMenuWidget::GoToTitle()
{
    // 타이틀로 돌아가는 로직
    //UGameplayStatics::OpenLevel(
    //    this,
    //    FName(*ProgressSaveGameInstance->Title));
}

void UGTWPauseMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 세팅
    IGTWWidgetInterface* HUDPawn = GetOwningPlayerPawn<IGTWWidgetInterface>();
    if (nullptr != HUDPawn)
    {
        HUDPawn->SetPauseMenuWidget(this);
    }
}


void UGTWPauseMenuWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication::Get()
            .OnApplicationActivationStateChanged()
            .AddUObject(
                this,
                &UGTWPauseMenuWidget::UnfocusMenu
            );
    }

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
