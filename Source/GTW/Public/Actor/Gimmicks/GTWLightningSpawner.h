// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GTWLightningSpawner.generated.h"

UCLASS()
class GTW_API AGTWLightningSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGTWLightningSpawner();

	UFUNCTION(BlueprintCallable, Category = "Boss|Gimmick|Lightning")
	int32 SpawnLightning(int32 SpawnCount);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Gimmick|Lightning", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AGTWLightningGimmick> LightningClass;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Gimmick|Lightning", meta=(AllowPrivateAccess = "true", MakeEditWidget = "true"))
	TArray<FVector> SpawnPositions;
};
