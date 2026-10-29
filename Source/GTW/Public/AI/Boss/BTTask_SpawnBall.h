// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SpawnBall.generated.h"

UCLASS()
class GTW_API UBTTask_SpawnBall : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_SpawnBall();

private:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
