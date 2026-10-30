// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GTWStartMoveSplineActor.generated.h"

UCLASS()
class GTW_API AGTWStartMoveSplineActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGTWStartMoveSplineActor();

	// 스플라인 움직임 시작되는 커스텀 함수.
	// 스플라인 BP에 시작 노드로 연결해놓은 후 GTWBoss 에서 호출하면 이동 시작.  
	UFUNCTION(BlueprintImplementableEvent, Category = Movement)
	void StartMovement();

};
