#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_DropObstacles.generated.h"

UCLASS()
class GTW_API UBTTask_DropObstacles : public UBTTaskNode
{
    GENERATED_BODY()
public:
    UBTTask_DropObstacles();
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
