// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GTWShootingGameManager.generated.h"

class UGTWGameInstance;
UCLASS()
class GTW_API AGTWShootingGameManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGTWShootingGameManager();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category = "Gameplay|Timer")
	float GetElapsedTime() const { return ElapsedTime; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetGameplayTimerRunning() const { return bIsGameplayTimerRunning; }
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetShootingClear() const { return bIsShootingClear; }
	UFUNCTION(BlueprintCallable)
	void ShootingGameStart();
	UFUNCTION(BlueprintCallable)
	void Reset();
	UFUNCTION(BlueprintCallable)
	void GameClear();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ActorStartLocation)
	FVector StartPlayerLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ActorStartLocation)
	FVector StartBossLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ActorStartLocation)
	FVector StartForwardMovingLocation;
	UPROPERTY(EditInstanceOnly, Category = "Shooting|Actor")
	TObjectPtr<AActor> StartLocationActor;
	UPROPERTY(EditInstanceOnly, Category = "Shooting|Actor")
	TObjectPtr<AActor> ShootingBossActor;
	UPROPERTY(EditInstanceOnly, Category = "Shooting|Actor")
	TObjectPtr<AActor> ForwardMovingTriggerActor;
	UPROPERTY(EditInstanceOnly, Category = "Shooting|Actor")
	TObjectPtr<AActor> ShootingBoxTriggerActor;

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
	bool bIsShootingClear = false;
	UGTWGameInstance* GameInstance;
	APlayerController* Player;
};
