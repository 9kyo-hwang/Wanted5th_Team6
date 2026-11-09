// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GTWProgressLevelSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class GTW_API UGTWProgressLevelSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ProgressLevel")
	FString ProgressLevel;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ProgressLevel")
	int32 ProgressPlayTime;
};
