// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GTWPauseMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class GTW_API UGTWPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UGTWPauseMenuWidget(const FObjectInitializer& ObjectInitializer);


public:
	// 창이 포커스를 잃었을 때 메뉴 열기 
	void UnfocusMenu();

	// ESC를 눌렀을 때 메뉴 토글 및 게임으로 돌아가기 버튼 누름.
	bool ToggleMenu();

	// 설정 버튼을 눌렀을 때 설정 창 토글
	void ToggleSetting();

	// TOOD: 사운드 조절 함수 작성


	// 타이틀로 돌아가기
	void GoToTitle();

protected:
	virtual void NativeConstruct() override;

	virtual void NativeOnInitialized() override;


protected:
	// 이 위젯을 소유하는 액터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Actor)
	TObjectPtr<AActor> OwningActor;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> PauseMenu;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> ResumeGame;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Setting;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Title;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> SettingMenu;

	UPROPERTY()
	bool bIsVisiblePauseMenu;

	UPROPERTY()
	bool bIsVisibleSettingMenu;


};
