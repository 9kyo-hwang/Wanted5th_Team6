// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GTWSpeedWidget.generated.h"

/**
 * 
 */
UCLASS()
class GTW_API UGTWSpeedWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UGTWSpeedWidget(const FObjectInitializer& ObjectInitializer);

	// 속도 변화에 반응할 이벤트
	void UpdateSpeed(int Speed);

protected:
	virtual void NativeConstruct() override;

	virtual void NativeOnInitialized() override;

protected:
	// 이 위젯을 소유하는 액터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Actor)
	TObjectPtr<AActor> OwningActor;
	
	// 속도 게이지 박스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHorizontalBox> SpeedGaugeBox;

	// 속도 게이지 배열
	UPROPERTY(Transient)
	TArray<TObjectPtr<class UImage>> SpeedGauges;

	// 속도 텍스트 박스
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> SpeedBox;

	// 속도 텍스트
	UPROPERTY(Transient)
	TArray<TObjectPtr<class UTextBlock>> SpeedText;


};
