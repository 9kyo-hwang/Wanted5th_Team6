// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GTWGameInstance.h"
#include "GTWClearHistorySaveGame.generated.h"

/**
 * 
 */
UCLASS()
class GTW_API UGTWClearHistorySaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Clear History")
	TMap<ELevelClear, double> LevelClearTime;
	//TArray<double> ClearTimes;	// 누적된 클리어 기록들
};
