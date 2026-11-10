#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_TornadoKick.generated.h"

/** 보스에게 킥을 요청하고 착지 모션이 끝날 때까지 다음 공격을 기다리게 한다. */
UCLASS()
class GTW_API UBTTask_TornadoKick : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_TornadoKick();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
