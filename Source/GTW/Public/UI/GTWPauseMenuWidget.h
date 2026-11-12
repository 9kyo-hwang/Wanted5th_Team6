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
	void UnfocusMenu(bool bIsActive);

	// ESC를 눌렀을 때 메뉴 토글 및 게임으로 돌아가기 버튼 누름.
	bool ToggleMenu();

	// TODO: 사운드 조절 함수 작성
	

	// 타이틀로 돌아가기
	void GoToTitle();

protected:
	virtual void NativeOnInitialized() override;

protected:
	// 버튼 클릭
	UFUNCTION()
	void ResumeClicked();

	UFUNCTION()
	void SettingClicked();

	UFUNCTION()
	void TitleClicked();

	// 슬라이더 변화
	UFUNCTION()
	void OnBGMVolumeChanged(float Value);

	UFUNCTION()
	void OnSFXVolumeChanged(float Value);

protected:
	// 게임 레벨 전환을 위한 인스턴스 참조
	UPROPERTY()
	TObjectPtr<class UGTWGameInstance> ProgressSaveGameInstance;

	// 이 위젯을 소유하는 액터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Actor)
	TObjectPtr<AActor> OwningActor;

	// 메뉴 캔버스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> PauseMenu;

	// 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> ResumeGame;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Setting;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Title;

	// 세팅 캠버스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> SettingMenu;

	// 볼륨 슬라이더
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class USlider> BGMSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> SFXSlider;

	// 사운드 믹서
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<class USoundMix> SettingsSoundMix;

	// 사운드 클래스 // 사운드 구분을 위한 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<class USoundClass> BGMSoundClass;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundClass> SFXSoundClass;

	UPROPERTY()
	bool bIsVisiblePauseMenu;

	UPROPERTY()
	bool bIsVisibleSettingMenu;

};
