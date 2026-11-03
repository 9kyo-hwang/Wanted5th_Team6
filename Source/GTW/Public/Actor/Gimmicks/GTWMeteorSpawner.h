// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GTWMeteorSpawner.generated.h"

UCLASS()
class GTW_API AGTWMeteorSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AGTWMeteorSpawner();

	UFUNCTION(BlueprintCallable, Category="Boss|Gimmick|Meteor")
	int32 SpawnMeteor(int32 SpawnCount);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Gimmick|Meteor", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AGTWMeteorGimmick> MeteorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Gimmick|Meteor", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	TArray<FVector> SpawnPositions;
};
