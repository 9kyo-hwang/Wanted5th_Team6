// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GTWPauseMenuWidget.h"
#include <Interfaces/GTWWidgetInterface.h>
#include <Components/CanvasPanel.h>

UGTWPauseMenuWidget::UGTWPauseMenuWidget(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{
    bIsVisiblePauseMenu = false;
}

void UGTWPauseMenuWidget::UnfocusMenu()
{

}

bool UGTWPauseMenuWidget::ToggleMenu()
{
    // 메뉴 창이 켜져 있을 때
    if (bIsVisiblePauseMenu)
    {
        PauseMenu.Get()->SetVisibility(ESlateVisibility::Collapsed);
        if (IsValid(SettingMenu))
        {
            SettingMenu->SetVisibility(ESlateVisibility::Collapsed);
        }

    }
    // 메뉴 창이 꺼져 있을 때
    else
    {
        PauseMenu.Get()->SetVisibility(ESlateVisibility::Visible);
    }
    bIsVisiblePauseMenu = !bIsVisiblePauseMenu;

    return bIsVisiblePauseMenu;

}

void UGTWPauseMenuWidget::ToggleSetting()
{
}

void UGTWPauseMenuWidget::GoToTitle()
{
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
}
