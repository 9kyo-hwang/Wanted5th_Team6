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

protected:
	// 위젯 클래스 정보.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HUD)
	TSubclassOf<class UGTWSpeedWidget> GTWSpeedWidgetClass;

	// 생성한 위젯을 저장할 변수.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = HUD)
	TObjectPtr<UGTWSpeedWidget> GTWSpeedWidget;

};
