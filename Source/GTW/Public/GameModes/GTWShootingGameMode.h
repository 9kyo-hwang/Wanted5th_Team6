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

public:
	UFUNCTION()
	void Reset();
	UFUNCTION()
	void GameOver();
private:
private:
	UPROPERTY(BlueprintReadWrite, Category = ActorStartLocation)
	FVector StartPlayerLocation;
	UPROPERTY(BlueprintReadWrite, Category = ActorStartLocation)
	FVector StartBossLocation;
	UPROPERTY(BlueprintReadWrite, Category = ActorStartLocation)
	FVector StartForwardMovingLocation;

protected:
protected:
};
