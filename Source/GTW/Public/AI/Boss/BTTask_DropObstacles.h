#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_DropObstacles.generated.h"

/** P2 공격 Task와 같은 즉시 완료형 Task. Boss의 PlayDropObstacles를 호출하고 성공을 반환한다. */
UCLASS()
class GTW_API UBTTask_DropObstacles : public UBTTaskNode
{
    GENERATED_BODY()
public:
    UBTTask_DropObstacles();
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
