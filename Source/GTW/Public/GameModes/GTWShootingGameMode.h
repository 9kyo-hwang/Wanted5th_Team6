// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GTWShootingGameMode.generated.h"

/**
 * 
 */
UCLASS()
class GTW_API AGTWShootingGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AGTWShootingGameMode();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "Gameplay|Timer")
	float GetElapsedTime() const { return ElapsedTime; }

	UFUNCTION(BlueprintCallable)
	void Reset();
	UFUNCTION(BlueprintCallable)
	void GameClear();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ActorStartLocation)
	FVector StartPlayerLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ActorStartLocation)
	FVector StartBossLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ActorStartLocation)
	FVector StartForwardMovingLocation;

	UPROPERTY(EditDefaultsOnly, Category = "Shooting|UI")
	TSubclassOf<UUserWidget> ElapsedWidgetClass;
	UPROPERTY()
	TObjectPtr<UUserWidget> ElapsedWidget;
	UPROPERTY(EditDefaultsOnly, Category = "Shooting|UI")
	TSubclassOf<UUserWidget> ClearViewWidgetClass;
	UPROPERTY()
	TObjectPtr<UUserWidget> ClearViewWidget;
private:
	void ShowElapsedWidget();
	void ShowClearViewWidget();
private:
	float ElapsedTime = 0.0f;
	bool bIsGameplayTimerRunning = false;
	APlayerController* Player;
protected:
};
