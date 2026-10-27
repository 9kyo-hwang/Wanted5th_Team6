// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_Lightning.generated.h"

/**
 * 
 */
UCLASS()
class GTW_API UBTT_Lightning : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_Lightning();

private:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;


};
