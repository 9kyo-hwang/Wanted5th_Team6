// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "DrawDebugHelpers.h"

#include "GTWGeometryCollectionTable.generated.h"

/**
 * 
 */
UCLASS()
class GTW_API AGTWGeometryCollectionTable : public AGeometryCollectionActor
{
	GENERATED_BODY()
public:
	AGTWGeometryCollectionTable() = default;

	virtual void Tick(float deltaTime) override;
	//void OnActorOverlap();
protected:
	// 특정 캐릭터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Class)
	TSubclassOf<AActor> TargetClass;
};
