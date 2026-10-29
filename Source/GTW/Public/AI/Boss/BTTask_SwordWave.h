// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SwordWave.generated.h"

/**
 * 
 */
UCLASS()
class GTW_API UBTTask_SwordWave : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_SwordWave();

private:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};
