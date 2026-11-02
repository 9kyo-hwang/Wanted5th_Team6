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

	//UFUNCTION()
	//void Reset();
	//UFUNCTION()
	//void GameOver();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ActorStartLocation)
	FVector StartPlayerLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ActorStartLocation)
	FVector StartBossLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ActorStartLocation)
	FVector StartForwardMovingLocation;
private:
private:

protected:
protected:
};
