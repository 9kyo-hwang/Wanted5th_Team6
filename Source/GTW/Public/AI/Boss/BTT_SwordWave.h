// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_SwordWave.generated.h"

/**
 * 
 */
UCLASS()
class GTW_API UBTT_SwordWave : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_SwordWave();

private:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};
